#include "evapcooler.h"

EvapCooler::EvapCooler(QString n, shared_ptr<ThermalSensor> s, shared_ptr<BasicOnOff> p, shared_ptr<BasicOnOff> f, int fanD) : BasicOnOff(n)
{
    sensor = s;
    pump = p;
    fanLow = f;
    fanMed = make_shared<VirtualIO>("Virtual Med Fan");
    fanHigh = make_shared<VirtualIO>("Virtual High Fan");
    purgePump = make_shared<VirtualIO>("Virtual Purge Pump");
    fanDelay = fanD;
    connect(sensor.get(), &ThermalSensor::tempChanged, this, &EvapCooler::tempChanged);
    purgeIntervalTimer = make_shared<QTimer>();
    purgeInterval = 0;
    purgeTimer = make_shared<QTimer>();
    purgeTimer->setSingleShot(true);
    startupTimer = make_shared<QTimer>();
    startupTimer->setSingleShot(true);
    connect(startupTimer.get(), &QTimer::timeout, this, &EvapCooler::startFan);

    filterTime = std::numeric_limits<int>::min();
    filterReminder = std::numeric_limits<int>::max();
    state = BasicOnOff::STATE::OFF;
    mode = Mode::Normal;
}

QString EvapCooler::getStatus() const
{
    QString ret = BasicOnOff::getStatus();
    ret += "\tMode: " + QString(QMetaEnum::fromType<EvapCooler::Mode>().valueToKey(mode)) + "\n";
    ret += "\tCurrent Temp: " + ThermalSensor::getFormatedTemp(sensor->getLastRead()) + "\n";
    ret += "\tControlled Devices: \n";
    ret += "\t" + pump->getStatus().replace("\t", "\t\t");
    ret += "\t" + fanLow->getStatus().replace("\t", "\t\t");
    if(isMedEnabled){
        ret += "\t" + fanMed->getStatus().replace("\t", "\t\t");
        ret += "\tWhen Temp:" + ThermalSensor::getFormatedTemp(tempMed) + "\n";
    }
    if(isHighEnabled){
        ret += "\t" + fanHigh->getStatus().replace("\t", "\t\t");
        ret += "\tWhen Temp:" + ThermalSensor::getFormatedTemp(tempHigh) + "\n";
    }
    if(purgeInterval > 0){
        ret += "\tPurge Interval: " + QString::number(purgeInterval / 1000) + "m\n";
        ret += "\tPurge Time: " + QString::number(purgeTime / 1000) + "s\n";
        ret += "\t" + purgePump->getStatus().replace("\t", "\t\t");
    }
    if(filterTime > std::numeric_limits<int>::min() && filterReminder < std::numeric_limits<int>::max()){
        double usedPercent = (double)filterReminder / (double)filterTime;
        ret += "\tFilter: %" + QString::number(usedPercent * 100) + "\n";
    }
    return ret;
}

void EvapCooler::setMedCondition(shared_ptr<BasicOnOff> f, double temp)
{
    fanMed = f;
    tempMed = temp;
    isMedEnabled = true;
}

void EvapCooler::setHighCondition(shared_ptr<BasicOnOff> f, double temp)
{
    fanHigh = f;
    tempHigh = temp;
    isHighEnabled = true;
}

void EvapCooler::turnOn()
{
    emit statusMessage(baseMessage + "Turn On");
    if(mode == Mode::Normal || mode == Mode::NormalMed || mode == Mode::Passive){
        pump->turnOn();
    }
    if(mode == Mode::Normal || mode == Mode::NormalMed){
        startupTimer->start(fanDelay * 1000);
    }
    if(mode == Mode::FanOnly){
        startupTimer->start(500);
    }
    state = BasicOnOff::STATE::ON;
}

void EvapCooler::turnOff()
{
    emit statusMessage(baseMessage + "Turn Off");
    setFan(fanSpeed::OFF);
    pump->turnOff();
    if(purgeIntervalTimer->isActive()){
        purgeIntervalTimer->stop();
        purgeNow();
    }
    state = BasicOnOff::OFF;
    QDateTime now = QDateTime::currentDateTime();
    int runTime = now.toSecsSinceEpoch() - startTime;
    filterTime += runTime;
    if(filterTime > filterReminder){
        emit filterIsDue();
        emit statusMessage("Filter due");
    }
}

void EvapCooler::setAuto()
{

}

void EvapCooler::tempChanged(double temp)
{
    if(state != BasicOnOff::STATE::OFF){
        if(isMedEnabled){
            if(fanState == fanSpeed::LOW && (temp - tempMed) > 0.5){
                setFan(fanSpeed::MED);
            }
            else if(fanState == fanSpeed::MED && (temp - tempMed) < -0.5){
                setFan(fanSpeed::LOW);
            }
        }
        if(isHighEnabled){
            if(fanState == fanSpeed::MED && (temp - tempHigh) > 0.5){
                setFan(fanSpeed::HIGH);
            }
            else if(fanState == fanSpeed::HIGH && (temp - tempHigh) < -0.5){
                setFan(fanSpeed::MED);
            }
        }
    }
}

void EvapCooler::setFan(fanSpeed f)
{
    QString msg = "Set Fan to " + QString(QMetaEnum::fromType<EvapCooler::fanSpeed>().valueToKey(f));
    emit statusMessage(baseMessage + msg);
    fanLow->turnOff();
    if(isMedEnabled){
        fanMed->turnOff();
    }
    if(isHighEnabled){
        fanHigh->turnOff();
    }
    fanState = f;
    switch (f) {
    case OFF:
        break;
    case LOW:
        fanLow->turnOn();
        break;
    case MED:
        if(isMedEnabled){
            fanMed->turnOn();
        }
        break;
    case HIGH:
        if(isHighEnabled){
            fanHigh->turnOn();
        }
        break;
    }
    fanState = f;
}

void EvapCooler::purgeNow()
{
    emit statusMessage("Purging");
    purgePump->turnOn();
    purgeTimer->start(purgeTime);
}

void EvapCooler::startFan()
{
    if(mode == Mode::NormalMed){
        setFan(fanSpeed::MED);
    }
    else{
        setFan(fanSpeed::LOW);
    }
    QDateTime now = QDateTime::currentDateTime();
    startTime = now.toSecsSinceEpoch();
    if(purgeInterval > 0 && mode != Mode::FanOnly){
        purgeIntervalTimer->start();
    }
}

bool EvapCooler::setMode(const string &newMode)
{
    bool valid = false;
    QMetaEnum allMode = QMetaEnum::fromType<EvapCooler::Mode>();
    Mode m = static_cast<Mode>(allMode.keyToValue(newMode.c_str(), &valid));
    if(valid){
        mode = m;
    }
    return valid;
}

string EvapCooler::lsModes()
{
    string ret;
    QMetaEnum allMode = QMetaEnum::fromType<EvapCooler::Mode>();
    for(int i = 0; i < allMode.keyCount(); i ++){
        ret += allMode.valueToKey(i);
        ret += "\n";
    }

    return ret;
}

int EvapCooler::getFilterTime() const
{
    QDateTime now = QDateTime::currentDateTime();;
    int add = 0;
    if(state == BasicOnOff::STATE::ON){
        add = now.toSecsSinceEpoch() - startTime;
    }
    return filterTime + add;
}

void EvapCooler::setFilterTime(int newFilterTime, int filterReminde)
{
    filterTime = newFilterTime;
    filterReminder = filterReminde;
}

void EvapCooler::setPurgePump(const shared_ptr<BasicOnOff> &newPurgePump, const int newPurgeTime, const int newPurgeInterval)
{
    purgePump = newPurgePump;
    purgeTime = newPurgeTime * 1000;
    purgeInterval = newPurgeInterval * 60 * 1000;
    purgeIntervalTimer->setInterval(purgeInterval);
    connect(purgeTimer.get(), &QTimer::timeout, purgePump.get(), &BasicOnOff::turnOff);
    connect(purgeIntervalTimer.get(), &QTimer::timeout, this, &EvapCooler::purgeNow);
}

