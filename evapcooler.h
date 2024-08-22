#ifndef EVAPCOOLER_H
#define EVAPCOOLER_H

#include "basiconoff.h"
#include "thermalsensor.h"
#include "virtualio.h"
#include <memory>
#include <QTimer>
#include <QDateTime>

class EvapCooler : public BasicOnOff
{
    Q_OBJECT
public:
    enum fanSpeed{
        OFF,
        LOW,
        MED,
        HIGH
    };
    Q_ENUM(fanSpeed)

    enum Mode{
        Normal,
        FanOnly,
        Passive,
        NormalMed
    };
    Q_ENUM(Mode);
    EvapCooler(QString n, shared_ptr<ThermalSensor> s, shared_ptr<BasicOnOff> p, shared_ptr<BasicOnOff> f, int fanD);
    virtual QString getStatus() const override;
    void setMedCondition(shared_ptr<BasicOnOff> f, double temp);
    void setHighCondition(shared_ptr<BasicOnOff> f, double temp);
    void setPurgePump(const shared_ptr<BasicOnOff> &newPurgePump, const int newPurgeTime, const int newPurgeInterval);
    void setFilterTime(int newFilterTime, int filterReminde);
    int getFilterTime() const;
    void purgeNow();
    bool setMode(const string &newMode);
    string lsModes();

    // Settable interface
    bool setOption(QString name, QVariant value) override;
    QJsonDocument lsOptions() override;

signals:
    void filterIsDue();
    void statusMessage(QString msg);

public slots:
    virtual void turnOn() override;
    virtual void turnOff() override;
    virtual void setAuto() override;

private:
    QString baseMessage = "EvapCooler: ";
    void tempChanged(double temp);
    void setFan(fanSpeed f);

    void startFan();

    shared_ptr<ThermalSensor> sensor;
    shared_ptr<BasicOnOff> pump;
    shared_ptr<BasicOnOff> purgePump;
    shared_ptr<BasicOnOff> fanLow;
    shared_ptr<BasicOnOff> fanMed;
    shared_ptr<BasicOnOff> fanHigh;

    shared_ptr<QTimer> purgeIntervalTimer;
    shared_ptr<QTimer> purgeTimer;
    shared_ptr<QTimer> startupTimer;

    double tempMed;
    double tempHigh;
    int purgeTime;
    int purgeInterval;

    unsigned long startTime;
    int filterTime;
    int filterReminder;

    int fanDelay;
    unsigned long runningTime;
    bool isMedEnabled = false;
    bool isHighEnabled = false;
    fanSpeed fanState;
    Mode mode;

};

#endif // EVAPCOOLER_H
