#ifndef THERMOSTAT_H
#define THERMOSTAT_H

#include <QObject>
#include <QDateTime>
#include <QMetaEnum>

#include <memory>

#include "thermalsensor.h"
#include "basiconoff.h"

using namespace std;

class Thermostat : public BasicOnOff
{
    Q_OBJECT
public:
    enum MODE{
        HEATING,
        COOLING
    };
    Q_ENUM(MODE);

    explicit Thermostat(QString n, shared_ptr<ThermalSensor> s, shared_ptr<BasicOnOff> device, double target, double threshold, MODE m);
    ~Thermostat();
    void setMinOffTime(int newMinOffTime);
    void setMinOnTime(int newMinOnTime);

    // Settable interface
    bool setOption(QString name, QVariant value) override;
    QJsonDocument lsOptions() override;

    // Statusable interface
    QJsonDocument jsonStatus() const override;
    QString getStatus() const override;

public slots:
    void turnOff() override;
    void turnOn() override;
    void setAuto() override;
    void setTemperature(double t);

signals:
    void onThermostatOn(QString name);
    void onThermostatOff(QString name);

private:
    void tempChanged(double temp);
    void requestOn();
    void requestOff();

    shared_ptr<ThermalSensor> sensor;
    shared_ptr<BasicOnOff> ioDevice;
    double targetTemp;
    double upper;
    double lower;
    double tempThreshold;
    int minOffTime;
    int minOnTime;
    unsigned long offTime;
    unsigned long onTime;
    MODE mode;
    bool isRunning;
    bool hold;

};

#endif // THERMOSTAT_H
