#ifndef CONSOLE_H
#define CONSOLE_H

#include <QObject>
#include <QSocketNotifier>
#include <QJsonDocument>
#include <iostream>
#include <QDebug>
#include <QProcess>
#include <QSettings>

using namespace std;

class Console : public QObject
{
    Q_OBJECT
public:
    explicit Console(QObject *parent = nullptr);
    ~Console();
    void run();

signals:
    void onQuit();
    void exit();
    void stop();
    void runTest();
    void sunRiseSetUpdated();
    bool addGpioOutput(QString name, int line, QString init, QString active);
    bool addThermalSensor(QString name, QString path, int interval, double threshold, double errorValue);
    void addNwsSensor(QString name, QString station, int interval, double threshold);
    bool addThermostat(QString name, QString sensor, QString ioDevice, double temperature, QString mode, double threshold, int minOn, int minOff);
    bool addThermalAlert(QString name, QString sensor, double threshold, double warning, double critical, bool isGreaterThan);
    bool addAlarm(QString name, QString time, QString ifDayNight);
    bool addOnOffGroup(QString name, QStringList devices);
    bool addEventAction(QString name, QString event, QString device, QString state, bool oneShot);
    bool addGpioInput(QString name, int line, QString edge, bool activeLow = false);
    bool addGpioPWM(QString name, QString outputDev, int rate);
    bool enableEvapCooler(bool enabled);
    bool configEvapCooler(QString name, QString sensor, QString pump, QString fan, int fanDelay);
    bool configEvapCoolerFanCondition(QString condition, QString fanName, double temp);
    bool configEvapCoolerPurgePump(QString pump, int purgeTime, int purgeInterval);
    bool configEvapCoolerFanFilter(int filterTime, int timeRemain);
    bool configEvapCoolerSetMode(QString mode);
    string lsBasicOnOff();
    string lsGpioConfig();
    string lsThermalSensor();
    string lsThermalSensorConfig();
    string lsSunRiseSet();
    string lsThermalAlerts();
    string lsEvents();
    string lsEventActions();
    string lsGpioInput();
    string lsEvapCooler();
    string lsEvapCoolerModes();
    QJsonDocument lsDeviceOptions();
    bool setDeviceOption(QString device, QString option, QVariant value);
    bool toggleRelay(QString name);

private:
    QSocketNotifier *notifier;
    QStringList lastCommand;

    bool findCommand(QStringList commandList);
    string setParse(QStringList commands);
    string configParse(QStringList commands);
    string configAddParse(QStringList commands);
    string configAddOutput(QStringList commands);
    string configAddThermal(QStringList commands);
    string configAddNwsThermal(QStringList commands);
    string configAddThermostat(QStringList commands);
    string configAddThermalAlert(QStringList commands);
    string configAddAlarm(QStringList commands);
    string configAddOnOffGroup(QStringList commands);
    string configAddEventAction(QStringList commands);
    string configAddGpioInput(QStringList commands);
    string configAddGpioPWM(QStringList commands);
    string ls(QStringList commands);
    string turnParse(QStringList commands);
    string toggleParse(QStringList commands);
    string commandHelp(QStringList commands);
    string globalParse(QStringList commands);
    string globalSetParse(QStringList commands);
    string globalSetThermalUnits(QStringList commands);
    string globalEvapCoolerParse(QStringList commands);
    string globalEvapCoolerEnable(QStringList commands);
    string globalEvapCoolerConfig(QStringList commands);
    string globalEvapCoolerFanCondition(QStringList commands);
    string globalEvapCoolerPurgePump(QStringList commands);
    string globalEvapCoolerFanFilter(QStringList commands);
    string lsGlobal();
    string lsConfig();
    string jsonToString(QJsonDocument jd);


    string OK = "ok";
    string FAIL = "failed";
    QString Setting_Latitude = "Latitude";
    QString Setting_Longitude = "Longitude";
    QString Setting_TimeZone = "TimeZone";


private slots:
    void readCommand();




};

#endif // CONSOLE_H
