#include "gpiopwm.h"

GpioPWM::GpioPWM(shared_ptr<GpioOutput> pin, QString n) : BasicOnOff(n){
    outputPin = pin;
    enabled = false;
    setLevel(100);
    this->workTimer = unique_ptr<QTimer>(new QTimer(this));
    //When Done working rest
    connect(workTimer.get(), &QTimer::timeout, this, &GpioPWM::onRest);
    workTimer->setSingleShot(true);

    this->restTimer = unique_ptr<QTimer>(new QTimer(this));
    // When Done resting work;
    connect(restTimer.get(), &QTimer::timeout, this, &GpioPWM::onWork);
    restTimer->setSingleShot(true);

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
    workTimer->setInterval(rate);
    restTimer->setInterval(100 - rate);

}

int GpioPWM::getRate() const
{
    return rate;
}

void GpioPWM::turnOn()
{
    if(!enabled && rate > 0){
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
    enabled = false;
    workTimer->stop();
    restTimer->stop();

}

QString GpioPWM::getStatus() const
{
    QString ret = BasicOnOff::getStatus();
    ret += "\tRate: " + QString::number(rate) + "\n";
    return ret;
}

void GpioPWM::onWork()
{
    if(enabled && rate > 0){
        outputPin->turnOn();
    }
    if(rate < 100){
        workTimer->start();
    }
}

void GpioPWM::onRest()
{
    outputPin->turnOff();
    if(enabled && rate > 0){
        restTimer->start();
    }
}
