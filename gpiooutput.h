#ifndef GPIOOUTPUT_H
#define GPIOOUTPUT_H

#include <QObject>
#include <QMetaEnum>
#include <QDebug>
#include <gpiod.hpp>
#include <memory>
#include <iostream>
#include <ostream>

#include "basiconoff.h"

using namespace std;

class GpioOutput : public BasicOnOff
{   
    Q_OBJECT
public:

    enum VALUE : int{
        LOW = 0,
        HIGH = 1
    };

    Q_ENUM(VALUE)

    GpioOutput(QString n, gpiod::line l, STATE init = STATE::OFF, VALUE on = VALUE::LOW);
    ~GpioOutput();
    void varDump();
    friend ostream& operator<<(ostream& os, const GpioOutput& obj);
    const QString &getName() const;
    int getCurrentValue() const;

    // Statusable interface
    QJsonDocument jsonStatus() const override;
    virtual QString getStatus() const override;


public slots:
    void turnOn() override;
    void turnOff() override;


private:
    unique_ptr<gpiod::line> line;


};

#endif // GPIOOUTPUT_H
