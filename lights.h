#ifndef LIGHTS_H
#define LIGHTS_H

#include "gpiooutput.h"
#include "basiconoff.h"
#include "gpiopwm.h"
#include "thermalsensor.h"
#include "sysfsthermalsensor.h"
#include "nwsthermalsensor.h"
#include "thermostat.h"
#include "thermalalert.h"
#include "sunriseset.h"
#include "alertlogger.h"
#include "statusable.h"
#include "actionevent.h"
#include "alarmevent.h"
#include "onoffgroup.h"
#include "eventtoaction.h"
#include "gpioinput.h"
#include "evapcooler.h"

#include <QThread>
#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QMetaEnum>
#include <QDateTime>
#include <gpiod.hpp>
#include <memory>

using namespace std;

class Lights : public QThread, public Settable
{
    Q_OBJECT
public:
    Lights(QObject *parent = nullptr);
    ~Lights();
    void run() override;
    void shutdown();

public slots:
    void runTest();

    bool turnOn(QString name);
    bool turnOff(QString name);
    bool turnAuto(QString name);
    bool toggle(QString name);
    bool setPWMRate(QString name, int rate);
    bool addGpioOutput(QString name, int line, QString init, QString act);
    bool addThermalSensor(QString name, QString path, int interval, double threshold, double errorValue);
    void addNwsSensor(QString name, QString station, int interval, double threshold);
    bool addThermostat(QString name, QString sensor, QString ioDevice, double temperature, QString mode, double threshold, int minOn, int minOff);
    bool addThermalAlert(QString name, QString sensor, double threshold, double warning, double critical, bool isGreaterThan);
    bool addGpioInput(QString name, int line, QString edge, bool activeLow = false);
    bool addGpioPWM(QString name, QString outputDev, int rate);
    bool configEvapCooler(QString name, QString sensor, QString pump, QString fan, int fanDelay);
    bool configEvapCoolerFanCondition(QString condition, QString fanName, double temp);
    bool configEvapCoolerPurgePump(QString pump, int purgeTime, int purgeInterval);
    bool configEvapCoolerFanFilter(int filterTime, int timeRemain);
    bool configEvapCoolerSetMode(QString mode);
    std::string lsBasicOnOff();
    std::string lsGpioConfig();
    std::string lsThermalSensor();
    std::string lsThermalSensorConfig();
    std::string lsSunRiseSet();
    std::string lsThermalAlerts();
    std::string lsEvents();
    std::string lsEventActions();
    std::string lsGpioInput();
    std::string lsEvapCooler();
    std::string lsEvapCoolerModes();
    QJsonDocument lsDeviceOptions();
    bool setDeviceOption(QString device, QString option, QVariant value);
    bool addAlarm(QString name, QString time, QString isDayNight);
    bool addOnOffGroup(QString name, QStringList devices);
    bool addEventAction(QString name, QString event, QString device, QString state, bool oneShot);
    bool setRelay(QString name, QString state);
    bool enableEvapCooler(bool enabled);
    bool setGpioChip(QString chipName);


private:
    QSettings settings;
    unique_ptr<SunRiseSet> sunRiseSet;
    unique_ptr<gpiod::chip> chip;
    shared_ptr<EvapCooler> evapCooler;
    shared_ptr<QHash<QString, shared_ptr<BasicOnOff>>> basicIOs;
    shared_ptr<QHash<QString, shared_ptr<ThermalSensor>>> thermalSensors;
    unique_ptr<QHash<QString, shared_ptr<Thermostat>>> thermostats;
    shared_ptr<QHash<QString, shared_ptr<ThermalAlert>>> thermalAlerts;
    shared_ptr<QHash<QString, shared_ptr<ActionEvent>>> events;
    shared_ptr<QHash<QString, shared_ptr<EventToAction>>> eventActions;
    shared_ptr<QHash<QString, shared_ptr<GpioInput>>> gpioInputs;
    shared_ptr<QHash<QString, shared_ptr<GpioPWM>>> gpioPWMs;
    unique_ptr<AlertLogger> consoleLogger;

    template<class T>
    string lsStatusable(shared_ptr<QHash<QString, shared_ptr<T>> > hash);

    void setupGpios();
    bool activateGpio(QString name, int line, int initial, int active);
    bool activateThermalSensor(QString name, QString path, int interval, double threshold, double errorValue);
    void activateNwsSensor(QString name, QString station, int interval, double threshold);
    bool activateThermostat(QString name, QString sensor, QString ioDevice, double temperature, QString mode, double threshold, int minOn, int minOff);
    bool activateThermalAlert(QString name, QString sensor, double threshold, double warning, double critical, bool isGreaterThan);
    bool activateAlarm(QString name, QString time, QString isDayNight);
    bool activateIOGroup(QString name, QStringList devices);
    bool activateEventToAction(QString name, QString event, QString state, QString device, bool oneShot);
    bool activateGpioInput(QString name, int line, QString edge, bool activeLow = false);
    bool activateEvapCooler(QString name, QString sensor, QString fan, QString pump, int fanDelay);
    bool activateEvapCoolerFanCondition(QString condition, QString fanName, double temp);
    bool activateEvapCoolerPurgePump(QString pump, int purgeTime, int purgeInterval);
    bool activateEvapCoolerFanFilter(int filterTime, int timeRemain);
    bool activateEvapCoolerSetMode(QString mode);
    bool activateGpioPwm(QString name, QString outputDev, int rate);

    void setupThermalSensors();
    void setupSunRiseSet();
    void setupNwsThermalSensor();
    void setupThermostat();
    void setupThermalAlerts();
    void setupEvents();
    void setupIOGroups();
    void setupEventToAction();
    void setupGpioInput();
    void setupEvapCooler();
    void setupPWMs();

    bool updateLocation(QVariant location);
    bool updateTimezone(QVariant zone);

    QString Setting_GPIO_Enabled;

    QString Setting_GPIO_Chip = "gpioChip";
    QString Setting_Name = "Name";
    QString Setting_Sunset = "Sunset";
    QString Setting_Sunrise = "Sunrise";
    QString Setting_SunriseIfAfterTime = "Conditional_Sunrise";
    QString Setting_SunsetIfBeforeTime = "Conditional_Sunset";

    QString Setting_NWS_Key = "NWS";
    QString Setting_NWS_Station = "station";

    QString Setting_GPIOOutputDevice = "gpioOutput";
    QString Setting_GPIOLineNo = "gpioLine";
    QString Setting_GPIOInitialState = "gpioInitial";
    QString Setting_GPIOActiveState = "gpioActive";

    QString Setting_ThermalSensor = "thermalSensor";
    QString Setting_ThermalPath = "thermalPath";
    QString Setting_ThermalInterval = "thermalInterval";
    QString Setting_ThermalThreshold = "thermalThreshold";
    QString Setting_ThermalErrorValue = "thermalErrorValue";

    QString Setting_Latitude = "Latitude";
    QString Setting_Longitude = "Longitude";
    QString Setting_TimeZone = "TimeZone";

    QString Setting_Thermostat = "Thermostats";
    QString Setting_Thermostat_Mode = "mode";
    QString Setting_Thermostat_Temperature = "temperature";
    QString Setting_Thermostat_MinOn = "minOn";
    QString Setting_Thermostat_MinOff = "minOff";

    QString Setting_ThermalAlert = "thermalAlerts";
    QString Setting_ThermalAlert_ThresholdTemp = "thresholdTemp";
    QString Setting_ThermalAlert_WarningTemp = "warningTemp";
    QString Setting_ThermalAlert_CriticalTemp = "criticalTemp";
    QString Setting_ThermalAlert_GreaterThan = "isGreaterThan";

    QString Setting_Alarm = "alarm";
    QString Setting_AlarmTime = "time";
    QString Setting_AlarmIsDay = "isDay";

    QString Setting_IOGroup = "ioGroup";
    QString Setting_IOGroupList = "devices";

    QString Setting_EventToAction = "eventToAction";
    QString Setting_EventToAction_Event = "event";
    QString Setting_EventToAction_Action = "action";
    QString Setting_EventToAction_State = "state";
    QString Setting_EventToAction_OneShot = "oneshot";

    QString Setting_GpioInput = "gpioInput";
    QString Setting_GpioEdge = "edge";
    QString Setting_GpioActiveLow = "activeLow";

    QString Setting_EvapCooler = "evapCooler";
    QString Setting_EvapCooler_Pump = "pump";
    QString Setting_EvapCooler_FanLow = "fanLow";
    QString Setting_EvapCooler_FanMed = "fanMed";
    QString Setting_EvapCooler_FanHigh = "fanHigh";
    QString Setting_EvapCooler_FanDelay = "fanDelay";
    QString Setting_EvapCooler_Purge = "purge";
    QString Setting_EvapCooler_PurgTime = "purgeTime";
    QString Setting_EvapCooler_PurgeInterval = "purgeInterval";
    QString Setting_EvapCooler_Filter = "filter";
    QString Setting_EvapCooler_FilterTime = "usedTime";
    QString Setting_EvapCooler_FilterLife = "lifeSpan";
    QString Setting_EvapCooler_Mode = "mode";

    QString Setting_GPIO_PWM = "gpioPWM";
    QString Setting_GPIO_PWM_Rate = "rate";

    // Settable interface
public:
    bool setOption(QString name, QVariant value) override;
    QJsonDocument lsOptions() override;
};

#endif // LIGHTS_H
