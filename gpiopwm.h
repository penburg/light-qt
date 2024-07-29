#ifndef GPIOPWM_H
#define GPIOPWM_H

#include <QObject>
#include <QTimer>
#include <memory>
#include <numeric>
#include "basiconoff.h"

using namespace std;

class GpioPWM :public BasicOnOff
{
    Q_OBJECT
public:
    explicit GpioPWM(shared_ptr<BasicOnOff> pin, QString n);
    ~GpioPWM();
    void setRate(int r);
    int getRate() const;


     // BasicOnOff interface
public slots:
    void turnOn() override;
    void turnOff() override;

    // Statusable interface
public:
    QString getStatus() const override;

private:
    int rate, workTime, restTime;
    bool enabled;
    void optomizeRate();
    void onWork();
    void onRest();
    shared_ptr<BasicOnOff> outputPin;
    unique_ptr<QTimer> workTimer;
    unique_ptr<QTimer> restTimer;




};

#endif // GPIOPWM_H
