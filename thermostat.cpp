#include "thermostat.h"

Thermostat::Thermostat(QString n, shared_ptr<ThermalSensor> s, shared_ptr<BasicOnOff> device, double target, double threshold, MODE m)  : BasicOnOff(n)
{
    //qDebug() << "Setup themeperature" << target << "threshold" << threshold;

    sensor = s;
    ioDevice = device;
    tempThreshold = threshold;
    setTemperature(target);

    mode = m;

    minOffTime = -1;
    minOnTime = -1;
    isRunning = false;
    hold = false;

    QDateTime now = QDateTime::currentDateTime();

    onTime = now.toSecsSinceEpoch() - 1;
    offTime = now.toSecsSinceEpoch();

    connect(sensor.get(), &ThermalSensor::tempChanged, this, &Thermostat::tempChanged);
    state = BasicOnOff::STATE::AUTO;
    tempChanged(sensor->getLastRead());
}

Thermostat::~Thermostat()
{
    ioDevice->turnOff();
}

void Thermostat::setMinOffTime(int newMinOffTime)
{
    minOffTime = newMinOffTime;
}

void Thermostat::tempChanged(double temp)
{

    //qDebug() << name << "Temp:" << temp << "target:" << targetTemp << "Upper:" << upper << "Lower:" << lower;

    if(hold){
        // do nothing
        //qInfo() << "Hold";
    }
    else if(mode == MODE::COOLING){
        if(isRunning){
            if(temp < lower){
                requestOff();
            }
        }
        else{
            if(temp > upper){
                requestOn();
            }
        }
    }
    else if (mode == MODE::HEATING){
        if(isRunning){
            if(temp > upper){
                requestOff();
            }
        }
        else{
            if(temp < lower){
                requestOn();
            }
        }
    }
}

void Thermostat::requestOn()
{
    //qInfo() << "requesting On";
    QDateTime now = QDateTime::currentDateTime();
    int diff = now.toSecsSinceEpoch() - offTime;
    if(diff > minOffTime){
        ioDevice->turnOn();
        isRunning = true;
        onTime = now.toSecsSinceEpoch();
        emit onThermostatOn(name);
    }
    else{
        qInfo() << "Unable to turn on, insufficient back off time diff:" << diff << "<=" << minOffTime;
    }
}

void Thermostat::requestOff()
{
   // qInfo() << "requesting Off";
    QDateTime now = QDateTime::currentDateTime();
    int diff = now.toSecsSinceEpoch() - onTime;
    if(diff > minOnTime){
        ioDevice->turnOff();
        isRunning = false;
        offTime = now.toSecsSinceEpoch();
        emit onThermostatOff(name);
    }
    else{
        qInfo() << "Unable to turn off, insufficient back off time diff:" << diff << "<=" << minOffTime;
    }
}

void Thermostat::setMinOnTime(int newMinOnTime)
{
    minOnTime = newMinOnTime;
}

void Thermostat::turnOff()
{
    hold = true;
    QDateTime now = QDateTime::currentDateTime();
    ioDevice->turnOff();
    isRunning = false;
    offTime = now.toSecsSinceEpoch();
    state = BasicOnOff::STATE::OFF;
}

void Thermostat::turnOn()
{
    hold = true;
    QDateTime now = QDateTime::currentDateTime();
    ioDevice->turnOn();
    isRunning = true;
    onTime = now.toSecsSinceEpoch();
    state = BasicOnOff::STATE::ON;
}

void Thermostat::setAuto()
{
    hold = false;
    state = BasicOnOff::STATE::AUTO;
}

QString Thermostat::getStatus() const
{
    QString ret = BasicOnOff::getStatus();
    ret += "\tMode: " + QString(QMetaEnum::fromType<Thermostat::MODE>().valueToKey(mode)) + "\n";
    ret += "\tSet Temp: " + ThermalSensor::getFormatedTemp(targetTemp) + " +- " + ThermalSensor::getFormatedValue(tempThreshold) + "\n";
    ret += "\tCurrent Temp: " + ThermalSensor::getFormatedTemp(sensor->getLastRead()) + "\n";
    ret += minOnTime > 0 ? "\tMinimum On: " + QString::number(minOnTime) + "\n": "";
    ret += minOffTime > 0 ? "\tMinimum Off: " + QString::number(minOffTime) + "\n": "";

    ret += "\tControlled Device: \n";
    ret += "\t" + ioDevice->getStatus().replace("\t", "\t\t");

    return ret;
}

void Thermostat::setTemperature(double t)
{
    targetTemp = t;
    upper = targetTemp + tempThreshold;
    lower = targetTemp - tempThreshold;

}
