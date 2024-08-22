#include "gpiopwm.h"

GpioPWM::GpioPWM(shared_ptr<BasicOnOff> pin, QString n) : BasicOnOff(n){
    outputPin = pin;
    enabled = false;
    state = STATE::OFF;

    this->workTimer = unique_ptr<QTimer>(new QTimer(this));
    this->workTimer->setTimerType(Qt::PreciseTimer);
    //When Done working rest
    connect(workTimer.get(), &QTimer::timeout, this, &GpioPWM::onRest);
    workTimer->setSingleShot(true);

    this->restTimer = unique_ptr<QTimer>(new QTimer(this));
    // When Done resting work;
    connect(restTimer.get(), &QTimer::timeout, this, &GpioPWM::onWork);
    restTimer->setSingleShot(true);
    this->restTimer->setTimerType(Qt::PreciseTimer);
    setRate(100);
}

GpioPWM::~GpioPWM()
{
    if(workTimer->isActive()){
        workTimer->stop();
    }
    if(restTimer->isActive()){
        restTimer->stop();
    }
    workTimer.reset();
    restTimer.reset();
}

void GpioPWM::setRate(int r)
{
    int level = r > 100 ? 100 : r;
    level = level < 0 ? 0 : level;
    rate = level;
    workTime = rate;
    restTime = 100 - rate;
    optomizeRate();
    workTimer->setInterval(workTime);
    restTimer->setInterval(restTime);
    if(enabled){//rest timer states
        GpioPWM::turnOff();
        GpioPWM::turnOn();
        enabled = true;
    }

}

int GpioPWM::getRate() const
{
    return rate;
}

void GpioPWM::turnOn()
{
    if(!enabled && workTime > 0){
        state = STATE::ON;
        enabled = true;
        if(rate < 100){
            onWork();
        }
        else{
            outputPin->turnOn();
        }
    }
}

void GpioPWM::turnOff()
{
    state = STATE::OFF;
    enabled = false;
    workTimer->stop();
    restTimer->stop();
    outputPin->turnOff();

}

QString GpioPWM::getStatus() const
{
    QString ret = BasicOnOff::getStatus();
    ret += "\tRate: " + QString::number(rate) + "\n";
    ret += "\tW/R time: " + QString::number(workTime) + "/" + QString::number(restTime) + "\n";
    ret += "read: " + QString::number(workTimer->interval()) + "/" + QString::number(restTimer->interval()) + "\n";
    return ret;
}

bool GpioPWM::setOption(QString name, QVariant value)
{
    bool isInt = false;
    int r = value.toInt(&isInt);
    if(name.compare("pwm", Qt::CaseInsensitive) == 0 && isInt){
        setRate(r);
        return true;
    }
    else{
        return false;
    }
}

QJsonDocument GpioPWM::lsOptions()
{
    QJsonArray ret = BasicOnOff::lsOptions().array();
    QVariantMap map;
    map.insert(keyName, "PWM");
    map.insert(keyValueType, "INT");
    map.insert(keyDesc, "Sets the PWM level to <0-100>%");

    ret.append(QJsonObject::fromVariantMap(map));
    return QJsonDocument(ret);
}

/* Reduces the work/rest time by it's gcd
*  if either time is 20 millisecs or more
*  we divide both by 5 so that they are and
*  reduce by the gcd again
*/
void GpioPWM::optomizeRate()
{
    int gcd = std::gcd(workTime, restTime);
    int optW = workTime / gcd;
    int optR = restTime / gcd;
    if(optW < 20 && optR  < 20){
        workTime = optW;
        restTime = optR;
    }
    else{
        optW = workTime / 5;
        optR = restTime / 5;
        gcd = std::gcd(optW, optR);
        workTime  = optW / gcd;
        restTime = optR / gcd;

    }
}

void GpioPWM::onWork()
{
    if(enabled && workTime > 0){
        outputPin->turnOn();
    }
    if(workTime < 100){
        workTimer->start();
    }
    if (workTime == 0){ //workTime = 0
        outputPin->turnOff();
    }
}

void GpioPWM::onRest()
{
    outputPin->turnOff();
    if(enabled && restTime > 0){
        restTimer->start();
    }
}
