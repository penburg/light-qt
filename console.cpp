#include "console.h"

Console::Console(QObject *parent) : QObject(parent)
{
    notifier = new QSocketNotifier(fileno(stdin), QSocketNotifier::Read, this);
}

Console::~Console()
{
    delete notifier;
    qDebug() << "Destroyed";
}

void Console::run() {
    std::cout << "First message" << std::endl;
    std::cout << "> " << std::flush;
    connect(notifier, SIGNAL(activated(int)), this, SLOT(readCommand()));
}

bool Console::findCommand(QStringList commandList)
{
    bool ret = false;
    QByteArray upHex = QByteArray::fromHex("1b5b41");
    QString upKey = QString::fromLocal8Bit((char*)upHex.data());

    if(commandList.at(0).compare("test", Qt::CaseSensitivity::CaseInsensitive) == 0){
        qInfo() << "Running Test";
        emit runTest();
        ret = true;
    }
    else if(commandList.at(0).compare("config", Qt::CaseSensitivity::CaseInsensitive) == 0){
        cout << configParse(commandList.mid(1)) << endl;
        ret = true;
    }
    else if(commandList.at(0).compare("ls", Qt::CaseSensitivity::CaseInsensitive) == 0){
        cout << ls(commandList.mid(1)) << endl;
        ret = true;
    }
    else if(commandList.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
        cout << commandHelp(commandList.mid(1)) << endl;
        ret = true;
    }
    else if(commandList.at(0).compare("turn", Qt::CaseSensitivity::CaseInsensitive) == 0){
        cout << turnParse(commandList.mid(1)) << endl;
        ret = true;
    }
    else if(commandList.at(0).compare("toggle", Qt::CaseSensitivity::CaseInsensitive) == 0){
        cout << toggleParse(commandList.mid(1)) << endl;
        ret = true;
    }
    else if(commandList.at(0).compare("globalConfig", Qt::CaseSensitivity::CaseInsensitive) == 0){
        cout << globalParse(commandList.mid(1)) << endl;
        ret = true;
    }
    else if(commandList.at(0).compare(upKey) == 0){
        if(lastCommand.size() > 0 && lastCommand.at(0).compare(upKey) != 0){
            ret = findCommand(lastCommand);
        }
    }

    else{
        cout << "Command not found \"" << commandList.at(0).toLocal8Bit().toHex().toStdString() << "\"" << endl;
    }
    return ret;

}

string Console::configParse(QStringList commands)
{
    if(commands.size() > 0){
        if(commands.at(0).compare("add", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddParse(commands.mid(1));
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "Avalable config Commands: add, help";
        }

        else{
            return "\"config help\" for useage";
        }
    }
    else{
        return "\"config help\" for useage";
    }

    return OK;
}

string Console::configAddParse(QStringList commands)
{
    if(commands.size() > 0){
        if(commands.at(0).compare("outputRelay", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddOutput(commands.mid(1));
        }
        else if(commands.at(0).compare("thermalSensor", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddThermal(commands.mid(1));
        }
        else if(commands.at(0).compare("thermalAlert", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddThermalAlert(commands.mid(1));
        }
        else if(commands.at(0).compare("thermostat", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddThermostat(commands.mid(1));
        }
        else if(commands.at(0).compare("NwsSensor", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddNwsThermal(commands.mid(1));
        }
        else if(commands.at(0).compare("alarm", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddAlarm(commands.mid(1));
        }
        else if(commands.at(0).compare("onoffgroup", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddOnOffGroup(commands.mid(1));
        }
        else if(commands.at(0).compare("eventAction", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddEventAction(commands.mid(1));
        }
        else if(commands.at(0).compare("GpioInput", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return configAddGpioInput(commands.mid(1));
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "Avalable config add Devices: thermalAlert, thermalSensor, thermostat, NwsSensor, outputRelay, alarm, OnOffGroup, GpioInput";
        }
    }
    return "\"config add help\" for useage";
}

string Console::configAddOutput(QStringList commands)
{
    if(commands.size() > 0){
        bool isNum;
        int line = commands.at(0).toInt(&isNum);
        if(isNum && commands.size() > 1){
            QString name = commands.at(1);
            QString initialState = commands.size() > 2 ? commands.at(2) : "off";
            QString active = commands.size() == 4 ? commands.at(3) : "low";
            bool status = emit addGpioOutput(name, line, initialState, active);
            return status ? OK : FAIL;
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "useage: config add outputRelay <line #> \"Name of Device\" <initialState=off> <active=low>";
        }
    }
    return "\"config add outputRelay help\" for useage";
}

string Console::configAddThermal(QStringList commands)
{
    if(commands.size() > 0){

        if(commands.size() > 1){
            QString name = commands.at(0);
            QString path = commands.at(1);
            bool isNum;
            int interval = 300;
            double threshold = 1.0;
            double errorValue = std::numeric_limits<double>::max();
            if(commands.size() > 2){
                int tmp = commands.at(2).toInt(&isNum);
                interval = isNum ? tmp : interval;
            }
            if(commands.size() > 3){
                double tmp = commands.at(3).toDouble(&isNum);
                threshold = isNum ? tmp : threshold;
            }
            if(commands.size() > 4){
                double tmp = commands.at(4).toDouble(&isNum);
                errorValue = isNum ? tmp : errorValue;
            }
            //qDebug() << "Setup Sensor:" << name << "path:" << path << "interval:" << interval << "threshold:" << threshold;
            bool status = emit addThermalSensor(name, path, interval, threshold, errorValue);
            return status ? OK : FAIL;
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "useage: config add thermalSensor \"Name of Device\" \"/path/to/sensor/therm\" <scanInterval=300> <reportThreshold=1.0>";
        }
    }
    return "\"config add thermalSensor help\" for useage";
}

string Console::configAddNwsThermal(QStringList commands)
{
    if(commands.size() > 0){

        if(commands.size() > 1){
            QString name = commands.at(0);
            QString station = commands.at(1);
            bool isNum;
            int interval = 3600;
            double threshold = 1.0;
            if(commands.size() > 2){
                interval = commands.at(2).toInt(&isNum);
                interval = isNum ? interval : 3600;
            }
            if(commands.size() > 3){
                threshold = commands.at(3).toDouble(&isNum);
                threshold = isNum ? threshold : 1.0;
            }
            //qDebug() << "Setup Sensor:" << name << "path:" << path << "interval:" << interval << "threshold:" << threshold;
            emit addNwsSensor(name, station, interval, threshold);
            return OK;
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "useage: config add NwsSensor \"Name of Device\" \"stationID\" <scanInterval=3600> <reportThreshold=1.0>";
        }
    }
    return "\"config add thermalSensor help\" for useage";
}

string Console::configAddThermostat(QStringList commands)
{
    string help = "useage: config add thermostat \"Name of Device\" <thermalSensor> <basicOnOfDevice> <temperature> <mode> <threshold=1.0> [optional]<minOn time> [optional]<minOff time>";
    if(commands.size() > 4){

        if(commands.size() > 1){
            QString name = commands.at(0);
            QString sensor = commands.at(1);
            QString ioDevice = commands.at(2);
            QString mode = commands.at(4);
            double temperature = std::numeric_limits<double>::max();
            bool isNum;
            int tmpInt;
            double tmpDbl;
            int minOn = std::numeric_limits<int>::max();
            int minOff = std::numeric_limits<int>::max();
            double threshold = 1.0;

            tmpDbl = commands.at(3).toDouble(&isNum);
            if(isNum){
                temperature = tmpDbl;
            }
            if(commands.size() > 5){
                double tmp = commands.at(5).toDouble(&isNum);
                threshold = isNum ? tmp : threshold;
            }

            if(commands.size() > 6){
                tmpInt = commands.at(6).toInt(&isNum);
                minOn = isNum ? tmpInt : minOn;
            }
            if(commands.size() > 7){
                tmpInt = commands.at(7).toInt(&isNum);
                minOff = isNum ? tmpInt : minOff;
            }

            bool status = emit addThermostat(name, sensor, ioDevice, temperature, mode, threshold, minOn, minOff);
            return status ? OK : FAIL;
        }

    }
    else if(commands.size() > 0 && commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
        return help;
    }
    return help;
}

string Console::configAddThermalAlert(QStringList commands)
{
    string help = "useage: config add thermalAlert \"Name of Alert\" \"NameOfThermalSensor\" [ < | gt | > | lt ] <AlertTemperature> (optional)<WarningTemperature> (optional)<CriticalTemperature>";

    if(commands.size() > 0){

        if(commands.size() > 3){
            QString name = commands.at(0);
            QString sensor = commands.at(1);
            bool isNum;
            double maxDoubleValue = std::numeric_limits<double>::max();
            double critical = maxDoubleValue;
            double warning = maxDoubleValue;
            double threshold = maxDoubleValue;
            bool greaterThan = false;

            QString gtValue = commands.at(2);
            if(gtValue.compare(">",Qt::CaseSensitivity::CaseInsensitive) == 0 || gtValue.compare("gt",Qt::CaseSensitivity::CaseInsensitive) == 0){
                greaterThan = true;
            }


            int tmp = commands.at(3).toDouble(&isNum);
            threshold = isNum ? tmp : threshold;


            if(commands.size() > 4){
                int tmp = commands.at(4).toDouble(&isNum);
                warning = isNum ? tmp : warning;
            }
            if(commands.size() > 5){
                double tmp = commands.at(5).toDouble(&isNum);
                critical = isNum ? tmp : critical;
            }
            //addThermalAlert(QString name, QString sensor, double threshold, double warning, double critical, bool isGreaterThan);
            bool status = emit addThermalAlert(name, sensor, threshold, warning, critical, greaterThan);
            return status ? OK : FAIL;
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return help;
        }
    }
    return help;
}

string Console::configAddAlarm(QStringList commands)
{
    string help = "useage: config add Alarm \"Name of Alarm\" <hh:mm> (optional)<ifDay|ifNight>";
    if(commands.size() >= 2){
        QString name = commands.at(0);
        QString time = commands.at(1);
        QString ifDayNight = "";
        if(commands.size() == 3){
            ifDayNight = commands.at(2);
        }
        bool status = emit addAlarm(name, time, ifDayNight);
        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::configAddOnOffGroup(QStringList commands)
{
    string help = "useage: config add OnOffGroup \"Name of group\" <device a> ... <device x>";
    if(commands.size() > 1){
        QString name = commands.at(0);
        bool status = emit addOnOffGroup(name, commands.mid(1));
        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::configAddEventAction(QStringList commands)
{
    string help = "useage: config add eventAction \"Name of eventAction\" <event> turn <on|off|auto> <device> <singleShot=false>";
    if(commands.size() >= 5 && commands.at(2).compare("turn", Qt::CaseSensitivity::CaseInsensitive) == 0){
        QString name = commands.at(0);
        QString event = commands.at(1);
        QString state = commands.at(3);
        QString device = commands.at(4);
        bool oneShot = false;
        if(commands.size() == 6){
            QString single = commands.at(5);
            if(single.compare("true", Qt::CaseSensitivity::CaseInsensitive) == 0 || single.compare("t", Qt::CaseSensitivity::CaseInsensitive) == 0){
                oneShot = true;
            }
        }

        bool status = emit addEventAction(name, event, device, state, oneShot);
        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::configAddGpioInput(QStringList commands)
{
    string help = "useage: config add GpioInput \"Name of Input\" <line #> <Riseing|Falling|Both> (optional)<activeLow>";
    if(commands.size() >= 3){
        bool isInt;
        QString name = commands.at(0);
        int line = commands.at(1).toInt(&isInt);
        QString edge = commands.at(2);
        bool status = false;
        bool activeLow = false;
        if(commands.size() == 4){
            if(commands.at(3).compare("activeLow", Qt::CaseSensitivity::CaseInsensitive) == 0){
                activeLow = true;
            }
        }
        if(isInt){
            status = emit addGpioInput(name, line, edge, activeLow);
        }
        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::ls(QStringList commands)
{
    if(commands.size() > 0){
        if(commands.at(0).compare("BasicOnOff", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsBasicOnOff();
        }
        else if(commands.at(0).compare("thermalSensor", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsThermalSensor();
        }
        else if(commands.at(0).compare("thermalAlert", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsThermalAlerts();
        }
        else if(commands.at(0).compare("globalConfig", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return lsGlobal();
        }
        else if(commands.at(0).compare("config", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return lsConfig();
        }
        else if(commands.at(0).compare("sunRiseSet", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsSunRiseSet();
        }
        else if(commands.at(0).compare("events", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsEvents();
        }
        else if(commands.at(0).compare("eventActions", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsEventActions();
        }
        else if(commands.at(0).compare("gpioInputs", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsGpioInput();
        }
        else if(commands.at(0).compare("evapCooler", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsEvapCooler();
        }
        else if(commands.at(0).compare("evapCoolerModes", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return emit lsEvapCoolerModes();
        }
        else{
            return "Not Found";
        }
    }
    else{
        return "config\nBasicOnOff\nsunRiseSet\nthermalAlert\nthermalSensor\nglobalConfig\nevents\neventActions\ngpioInputs\nevapCooler\nevapCoolerModes";
    }
}

string Console::turnParse(QStringList commands)
{
    if(commands.size() > 0){
        if(commands.at(0).compare("on", Qt::CaseSensitivity::CaseInsensitive) == 0 && commands.size() == 2){
            bool success = emit setRelay(commands.at(1), "ON");
            return success ? OK : FAIL;
        }
        else if(commands.at(0).compare("off", Qt::CaseSensitivity::CaseInsensitive) == 0 && commands.size() == 2){
            bool success = emit setRelay(commands.at(1), "OFF");
            return success ? OK : FAIL;
        }
        else if(commands.at(0).compare("auto", Qt::CaseSensitivity::CaseInsensitive) == 0 && commands.size() == 2){
            bool success = emit setRelay(commands.at(1), "AUTO");
            return success ? OK : FAIL;
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "useage: turn <on|off> \"RelayName\"";
        }
        else{
            return "\"turn help\" for useage";
        }
    }
    else{
        return "\"turn help\" for useage";
    }
}

string Console::toggleParse(QStringList commands)
{
    if(commands.size() > 0){
        if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "useage: toggle \"RelayName\"";
        }
        else{
            bool success = emit toggleRelay(commands.at(0));
            return success ? OK : FAIL;
        }
    }
    else{
        return "\"toggle help\" for useage";
    }
}

string Console::commandHelp(QStringList commands)
{
    if(commands.size() == 0){
        return "Avalable Commands: config, globalConfig, ls, toggle, turn <auto|on|off>, quit";
    }

    return "\"help\" for useage";
}

string Console::globalParse(QStringList commands)
{
    if(commands.size() > 0){
        if(commands.at(0).compare("set", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalSetParse(commands.mid(1));
        }
        else if(commands.at(0).compare("evapCooler", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalEvapCoolerParse(commands.mid(1));
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "Avalable globalConfig Commands: evapCooler, help, set";
        }

    }

    return "\"globalConfig help\" for useage";

}

string Console::globalSetParse(QStringList commands)
{
    if(commands.size() > 0){
        if(commands.at(0).compare("thermalUnits", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalSetThermalUnits(commands.mid(1));
        }
        else if(commands.at(0).compare("sunRiseSet", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalSetSunCalcs(commands.mid(1));
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return "Avalable globalConfig set options: thermalUnits, sunRiseSet";
        }
    }
    return "\"globalConfig set help\" for useage";
}

string Console::globalSetThermalUnits(QStringList commands)
{
    QSettings settings;
    if(commands.size() > 0){
        if(commands.at(0).compare("c", Qt::CaseSensitivity::CaseInsensitive) == 0 || commands.at(0).compare("celsius", Qt::CaseSensitivity::CaseInsensitive) == 0){
            settings.setValue("useFahrenheit", false);
            return "setting thermalUnits to celsius";
        }
        else if(commands.at(0).compare("f", Qt::CaseSensitivity::CaseInsensitive) == 0 || commands.at(0).compare("fahrenheit", Qt::CaseSensitivity::CaseInsensitive) == 0){
            settings.setValue("useFahrenheit", true);
            return "setting thermalUnits to fahrenheit";
        }
        else{
            return "useage: globalConfig set thermalUnits <c|celsius|f|fahrenheit>";
        }
    }
    return "\"globalConfig set thermalUnits help\" for useage";
}

string Console::globalSetSunCalcs(QStringList commands)
{
    QSettings settings;
    string useage = "useage: globalConfig set sunRiseSet <latitude = 40> <longitude = -105> <timeZone = -7>";
    if(commands.size() == 3){
        bool isLat, isLon, isTime;
        double lat = commands.at(0).toDouble(&isLat);
        double lon = commands.at(1).toDouble(&isLon);
        int time = commands.at(2).toDouble(&isTime);
        if(isLat && isLon && isTime){
            settings.setValue(Setting_Latitude, lat);
            settings.setValue(Setting_Longitude, lon);
            settings.setValue(Setting_TimeZone, time);
            emit sunRiseSetUpdated();
            return OK;
        }
        else{
            return useage;
        }

    }

    return useage;

}

string Console::globalEvapCoolerParse(QStringList commands)
{
    string help = "Avalable globalConfig evapCooler Commands: <enable|disable>, <config>, add<Fan|PurgePump>, setFanFilter, setMode";
    if(commands.size() > 0){
        if(commands.at(0).compare("enable", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalEvapCoolerEnable(commands.mid(0));
        }
        else if(commands.at(0).compare("disable", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalEvapCoolerEnable(commands.mid(0));
        }
        else if(commands.at(0).compare("config", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalEvapCoolerConfig(commands.mid(1));
        }
        else if(commands.at(0).compare("addFan", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalEvapCoolerFanCondition(commands.mid(1));
        }
        else if(commands.at(0).compare("addPurgePump", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalEvapCoolerPurgePump(commands.mid(1));
        }
        else if(commands.at(0).compare("setFanFilter", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalEvapCoolerFanFilter(commands.mid(1));
        }
        else if(commands.at(0).compare("setMode", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return globalEvapCoolerSetMode(commands.mid(1));
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return help;
        }

    }

    return help;
}

string Console::globalEvapCoolerEnable(QStringList commands)
{
    string help = "useage: globalConfig evapCooler <enable|disable>";
    if(commands.size() > 0){
        if(commands.at(0).compare("enable", Qt::CaseSensitivity::CaseInsensitive) == 0){
            bool status = emit enableEvapCooler(true);
            return status ? OK : FAIL;
        }
        else if(commands.at(0).compare("disable", Qt::CaseSensitivity::CaseInsensitive) == 0){
            bool status = emit enableEvapCooler(false);
            return status ? OK : FAIL;
        }
        else if(commands.at(0).compare("help", Qt::CaseSensitivity::CaseInsensitive) == 0){
            return help;
        }

    }

    return help;
}

string Console::globalEvapCoolerConfig(QStringList commands)
{
    string help = "useage: globalConfig evapCooler config \"Name Evap. Cooler\" \"Sensor\" \"Pump\" \"Fan\" <fanDelay=300>";
    if(commands.size() >= 4){
        bool isInt;
        QString name = commands.at(0);
        QString sensor = commands.at(1);
        QString pump = commands.at(2);
        QString fan = commands.at(3);
        int fanDelay = 300;
        if(commands.size() == 5){
            int tmp = commands.at(4).toInt(&isInt);
            if(isInt){
                fanDelay = tmp;
            }
        }

        bool status = emit configEvapCooler(name, sensor, pump, fan, fanDelay);

        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::globalEvapCoolerFanCondition(QStringList commands)
{
    string help = "useage: globalConfig evapCooler addFan <med|high> \"Fan\" <temperature>";
    if(commands.size() == 3){
        bool isDouble;
        QString mode = commands.at(0);
        QString fan = commands.at(1);
        int temperature = commands.at(2).toDouble(&isDouble);
        bool status = false;
        if(isDouble){
            status = emit configEvapCoolerFanCondition(mode, fan, temperature);
        }
        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::globalEvapCoolerPurgePump(QStringList commands)
{
    string help = "useage: globalConfig evapCooler addPurgePump \"Pump\" <purgeTime=60(s)> <purgeInterval=480(m)>";
    if(commands.size() >= 1){
        bool isInt;
        QString pump = commands.at(0);
        int purgeTime = 60;
        int purgeInterval = 480;
        if(commands.size() >= 2){
            int tmp = commands.at(1).toInt(&isInt);
            if(isInt){
                purgeTime = tmp;
            }
        }
        if(commands.size() == 3){
            int tmp = commands.at(2).toInt(&isInt);
            if(isInt){
                purgeInterval = tmp;
            }
        }

        bool status = emit configEvapCoolerPurgePump(pump, purgeTime, purgeInterval);
        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::globalEvapCoolerFanFilter(QStringList commands)
{
    string help = "useage: globalConfig evapCooler setFanFilter resetTo <filterUseTime=0(s)> <filterLifeExpectancy=200(h)>";
    if(commands.size() >= 1){
        bool isInt;
        int filterTime = 0;
        int filterLife = 200 * 3600;
        if(commands.size() >= 2){
            int tmp = commands.at(1).toInt(&isInt);
            if(isInt){
                filterTime = tmp;
            }
        }
        if(commands.size() == 3){
            int tmp = commands.at(2).toInt(&isInt);
            if(isInt){
                filterLife = tmp * 3600;
            }
        }
        bool status = false;
        if(commands.at(0).compare("resetTo", Qt::CaseSensitivity::CaseInsensitive) == 0){
            status = emit configEvapCoolerFanFilter(filterTime, filterLife);
        }
        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::globalEvapCoolerSetMode(QStringList commands)
{
    string help = "useage: globalConfig evapCooler setMode <mode>";
    if(commands.size() == 1){

        bool status = emit configEvapCoolerSetMode(commands.at(0));

        return status ? OK : FAIL;
    }
    else{
        return help;
    }
}

string Console::lsGlobal()
{
    QSettings settings;
    string ret = "";
    bool useF = settings.value("useFahrenheit", false).toBool();
    ret += "thermalUnits: ";
    ret += useF ? "fahrenheit" : "celsius" ;
    ret += + "\n";

    string lat = settings.value(Setting_Latitude, 40).toString().toStdString();
    string lon = settings.value(Setting_Longitude, -105).toString().toStdString();
    string timeZone = settings.value(Setting_TimeZone, -7).toString().toStdString();

    ret += "latitude: " + lat + "\n";
    ret += "longitude: " + lon + "\n";
    ret += "timeZone: " + timeZone + "\n";

    return ret;
}

string Console::lsConfig()
{
    string ret = "Config\n";
    ret += "GPIO Config:\n";
    ret += emit lsGpioConfig();
    ret += "Thermal Sensor Config:\n";
    ret += emit lsThermalSensorConfig();

    return ret;
}

void Console::readCommand() {
    std::string line;
    std::getline(std::cin, line);
    QString command(line.c_str());
    QStringList commands =  QProcess::splitCommand(command);
    QByteArray upHex = QByteArray::fromHex("1b5b41");
    QString upKey = QString::fromLocal8Bit((char*)upHex.data());

    if (std::cin.eof() || line == "quit") {
        qWarning() << "Good bye!";
        emit onQuit();

    }
    else if(commands.size() > 0){
        bool success = findCommand(commands);
        if(success && commands.at(0).compare(upKey) != 0){
            lastCommand = commands;
        }

    }
    else {
        qInfo() << "Command Not Found" << line.c_str();
        std::cout << "" << std::flush;
    }
}
