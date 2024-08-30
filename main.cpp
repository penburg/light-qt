#include <QCoreApplication>
#include <csignal>
#include <memory>
#include <stdio.h>
#include "console.h"
#include "errorhandler.h"
#include "apihandler.h"
#include "lights.h"
#include "alertlogger.h"

using namespace std;

shared_ptr<ApiHandler> api;
shared_ptr<Lights> backend;

void sigHandler(int s)
{
    std::signal(s, SIG_DFL);
    std::cout << "Emergency Shutdown" << std::endl;
    api->shutdown();
    if(backend->isRunning()){
        backend->shutdown();
    }
    qApp->quit();
    std::cout << "Application Quit" << std::endl;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(errorHandler);

    std::signal(SIGINT,  sigHandler);
    std::signal(SIGTERM, sigHandler);

    QCoreApplication a(argc, argv);
    QCoreApplication::setOrganizationName("Enburg Dev");
    QCoreApplication::setOrganizationDomain("enburg.dev");
    QCoreApplication::setApplicationName("Lights-QT");
    QCoreApplication::setApplicationVersion("0.0.1");

    Console console;
    console.run();

    api = make_shared<ApiHandler>();

    backend = make_shared<Lights>();
    backend->start();

    QObject::connect(&console, &Console::onQuit, &a, &QCoreApplication::quit);
    QObject::connect(&console, &Console::onQuit, backend.get(), &Lights::shutdown);
    QObject::connect(&console, &Console::onQuit, api.get(), &ApiHandler::shutdown);

    QObject::connect(&console, &Console::runTest, backend.get(), &Lights::runTest);
    QObject::connect(&console, &Console::addGpioOutput, backend.get(), &Lights::addGpioOutput);
    QObject::connect(&console, &Console::addThermalSensor, backend.get(), &Lights::addThermalSensor);
    QObject::connect(&console, &Console::addNwsSensor, backend.get(), &Lights::addNwsSensor);
    QObject::connect(&console, &Console::addThermostat, backend.get(), &Lights::addThermostat);
    QObject::connect(&console, &Console::addThermalAlert, backend.get(), &Lights::addThermalAlert);
    QObject::connect(&console, &Console::addAlarm, backend.get(), &Lights::addAlarm);
    QObject::connect(&console, &Console::addOnOffGroup, backend.get(), &Lights::addOnOffGroup);
    QObject::connect(&console, &Console::addEventAction, backend.get(), &Lights::addEventAction);
    QObject::connect(&console, &Console::addGpioInput, backend.get(), &Lights::addGpioInput);
    QObject::connect(&console, &Console::addGpioPWM, backend.get(), &Lights::addGpioPWM);

    QObject::connect(&console, &Console::enableEvapCooler, backend.get(), &Lights::enableEvapCooler);
    QObject::connect(&console, &Console::configEvapCooler, backend.get(), &Lights::configEvapCooler);
    QObject::connect(&console, &Console::configEvapCoolerFanCondition, backend.get(), &Lights::configEvapCoolerFanCondition);
    QObject::connect(&console, &Console::configEvapCoolerFanFilter, backend.get(), &Lights::configEvapCoolerFanFilter);
    QObject::connect(&console, &Console::configEvapCoolerPurgePump, backend.get(), &Lights::configEvapCoolerPurgePump);

    QObject::connect(&console, &Console::lsBasicOnOff, backend.get(), &Lights::lsBasicOnOff);
    QObject::connect(&console, &Console::lsThermalSensor, backend.get(), &Lights::lsThermalSensor);
    QObject::connect(&console, &Console::lsThermalAlerts, backend.get(), &Lights::lsThermalAlerts);
    QObject::connect(&console, &Console::lsGpioConfig, backend.get(), &Lights::lsGpioConfig);
    QObject::connect(&console, &Console::lsThermalSensorConfig, backend.get(), &Lights::lsThermalSensorConfig);
    QObject::connect(&console, &Console::lsSunRiseSet, backend.get(), &Lights::lsSunRiseSet);
    QObject::connect(&console, &Console::lsEvents, backend.get(), &Lights::lsEvents);
    QObject::connect(&console, &Console::lsEventActions, backend.get(), &Lights::lsEventActions);
    QObject::connect(&console, &Console::lsGpioInput, backend.get(), &Lights::lsGpioInput);
    QObject::connect(&console, &Console::lsEvapCooler, backend.get(), &Lights::lsEvapCooler);
    QObject::connect(&console, &Console::lsEvapCoolerModes, backend.get(), &Lights::lsEvapCoolerModes);

    QObject::connect(&console, &Console::setRelay, backend.get(), &Lights::setRelay);
    QObject::connect(&console, &Console::toggleRelay, backend.get(), &Lights::toggle);

    QObject::connect(&console, &Console::lsDeviceOptions, backend.get(), &Lights::lsDeviceOptions);
    QObject::connect(&console, &Console::setDeviceOption, backend.get(), &Lights::setDeviceOption);



    return a.exec();
}
