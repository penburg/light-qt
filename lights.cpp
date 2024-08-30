#include "lights.h"

Lights::Lights(QObject *parent) : QThread(parent)
{
    QString chipName = settings.value(Setting_GPIO_Chip, "gpiochip0").toString();

    try{
        chip = unique_ptr<gpiod::chip>(new gpiod::chip(chipName.toStdString()));
    }
    catch (std::system_error& e){
        qWarning() << "Gpio Chip ERROR" << e.code().value() << "-" << e.what();
    }

    basicIOs = make_shared<QHash<QString, shared_ptr<BasicOnOff>>>();
    thermalSensors = make_shared<QHash<QString, shared_ptr<ThermalSensor>>>();
    thermostats = unique_ptr<QHash<QString, shared_ptr<Thermostat>>>(new QHash<QString, shared_ptr<Thermostat>>());
    thermalAlerts = make_shared<QHash<QString, shared_ptr<ThermalAlert>>>();
    events = make_shared<QHash<QString, shared_ptr<ActionEvent>>>();
    eventActions = make_shared<QHash<QString, shared_ptr<EventToAction>>>();
    consoleLogger = unique_ptr<AlertLogger>(new AlertLogger());
    sunRiseSet = unique_ptr<SunRiseSet>(new SunRiseSet());
    gpioInputs = make_shared<QHash<QString, shared_ptr<GpioInput>>>();
    gpioPWMs = make_shared<QHash<QString, shared_ptr<GpioPWM>>>();
    setupGpios();
    setupThermalSensors();
    setupNwsThermalSensor();
    setupSunRiseSet();
    setupPWMs();
    connect(sunRiseSet.get(), &SunRiseSet::onSunRise, consoleLogger.get(), &AlertLogger::onSunRise);
    connect(sunRiseSet.get(), &SunRiseSet::onSunSet, consoleLogger.get(), &AlertLogger::onSunSet);

    setupEvapCooler();
    setupIOGroups();

    setupThermostat();
    setupThermalAlerts();
    setupGpioInput();
    setupEvents();
    setupEventToAction();



}

Lights::~Lights()
{

    if(!thermalSensors->empty()){
        qDebug() << "Shutting thermal sensors down";
        QHash<QString, shared_ptr<ThermalSensor>>::Iterator i;
        for (i = thermalSensors->begin(); i != thermalSensors->end(); ++i){
            i.value()->shutdown();
            qDebug() << i.key() << "Shutdown";
        }
    }
    if(!gpioPWMs->isEmpty()){
        gpioPWMs->clear();
    }
    if(!basicIOs->empty()){
        qDebug() << "Shuting gpios down";
        QHash<QString, shared_ptr<BasicOnOff>>::Iterator i;
        for (i = basicIOs->begin(); i != basicIOs->end(); ++i){
            i.value().reset();
            qDebug() << i.key() << "Shutdown";
        }

    }
    qDebug() << "Destroyed";

}

void Lights::run()
{
    qDebug() << "Starting Backend";
    QThread::exec();
}

void Lights::shutdown()
{
    qDebug() << "Stopping Backend";
    if(gpioInputs->size() > 0){
        QStringList keys = gpioInputs->keys();
        foreach(QString key, keys){
            auto value = gpioInputs->value(key);
            value->shutdown();
        }

    }
    exit(EXIT_SUCCESS);
}

void Lights::runTest()
{

}

bool Lights::turnOn(QString name)
{
    shared_ptr<BasicOnOff> pin = basicIOs->value(name, nullptr);
    if(pin != nullptr){
        pin->turnOn();
        return true;
    }
    else{
        qWarning() << name << "Not Found";
        return false;
    }

}

bool Lights::turnOff(QString name)
{
    shared_ptr<BasicOnOff> pin = basicIOs->value(name, nullptr);
    if(pin != nullptr){
        pin->turnOff();
        return true;
    }
    else{
        qWarning() << name << "Not Found";
        return false;
    }
}

bool Lights::turnAuto(QString name)
{
    shared_ptr<BasicOnOff> pin = basicIOs->value(name, nullptr);
    if(pin != nullptr){
        pin->setAuto();
        return true;
    }
    else{
        qWarning() << name << "Not Found";
        return false;
    }
}

bool Lights::toggle(QString name)
{
    shared_ptr<BasicOnOff> pin = basicIOs->value(name, nullptr);
    if(pin != nullptr){
        return pin->getState() == BasicOnOff::STATE::ON ? turnOff(name) : turnOn(name);
    }
    else{
        qWarning() << name << "Not Found";
        return false;
    }
}

bool Lights::setPWMRate(QString name, int rate)
{
    shared_ptr<GpioPWM> pwm = gpioPWMs->value(name, nullptr);
    if(pwm != nullptr){
        pwm->setRate(rate);
        return true;
    }
    else{
        qWarning()  << name << "Not Found";
        return false;
    }
}

bool Lights::addGpioOutput(QString name, int line, QString init, QString act)
{
    int initial = QMetaEnum::fromType<BasicOnOff::STATE>().keyToValue(init.toUpper().toLocal8Bit());
    int active = QMetaEnum::fromType<GpioOutput::VALUE>().keyToValue(act.toUpper().toLocal8Bit());
    bool success = activateGpio(name, line, initial, active);
    if(success){
        int size = settings.beginReadArray(Setting_GPIOOutputDevice);
        settings.endArray();

        settings.beginWriteArray(Setting_GPIOOutputDevice);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_GPIOLineNo, line);
        settings.setValue(Setting_GPIOInitialState, init.toUpper());
        settings.setValue(Setting_GPIOActiveState, act.toUpper());
        settings.endArray();

    }

    return success;
}

bool Lights::addThermalSensor(QString name, QString path, int interval, double threshold, double errorValue)
{
    //qDebug() << "Adding Sensor:" << name << "path:" << path << "interval:" << interval << "threshold:" << threshold;
    bool success = activateThermalSensor(name, path, interval, threshold, errorValue);
    if(success){
        int size = settings.beginReadArray(Setting_ThermalSensor);
        settings.endArray();

        settings.beginWriteArray(Setting_ThermalSensor);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_ThermalPath, path);
        settings.setValue(Setting_ThermalInterval, interval);
        settings.setValue(Setting_ThermalThreshold, threshold);
        if(errorValue != std::numeric_limits<double>::max()){
            settings.setValue(Setting_ThermalErrorValue, errorValue);
        }

        settings.endArray();

    }

    return success;
}

void Lights::addNwsSensor(QString name, QString station, int interval, double threshold)
{
    activateNwsSensor(name, station, interval, threshold);
    int size = settings.beginReadArray(Setting_NWS_Key);
    settings.endArray();

    settings.beginWriteArray(Setting_NWS_Key);
    settings.setArrayIndex(size);
    settings.setValue(Setting_Name, name);
    settings.setValue(Setting_NWS_Station, station);
    settings.setValue(Setting_ThermalInterval, interval);
    settings.setValue(Setting_ThermalThreshold, threshold);
    settings.endArray();
}

bool Lights::addThermostat(QString name, QString sensor, QString ioDevice, double temperature, QString mode, double threshold, int minOn, int minOff)
{
    bool success = activateThermostat(name, sensor, ioDevice, temperature, mode, threshold, minOn, minOff);
    if(success){
        int size = settings.beginReadArray(Setting_Thermostat);
        settings.endArray();

        settings.beginWriteArray(Setting_Thermostat);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_ThermalSensor, sensor);
        settings.setValue(Setting_GPIOOutputDevice, ioDevice);
        settings.setValue(Setting_Thermostat_Temperature, temperature);
        settings.setValue(Setting_ThermalThreshold, threshold);
        settings.setValue(Setting_Thermostat_Mode, mode);
        if(minOn < std::numeric_limits<int>::max()){
            settings.setValue(Setting_Thermostat_MinOn, minOn);
        }
        if(minOff < std::numeric_limits<int>::max()){
            settings.setValue(Setting_Thermostat_MinOff, minOff);
        }

        settings.endArray();
    }
    return success;
}

bool Lights::addThermalAlert(QString name, QString sensor, double threshold, double warning, double critical, bool isGreaterThan)
{
    bool success = activateThermalAlert(name, sensor, threshold, warning, critical, isGreaterThan);
    if(success){
        double maxDoubleValue = std::numeric_limits<double>::max();
        int size = settings.beginReadArray(Setting_ThermalAlert);
        settings.endArray();

        settings.beginWriteArray(Setting_ThermalAlert);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_ThermalSensor, sensor);
        settings.setValue(Setting_ThermalThreshold, threshold);
        settings.setValue(Setting_ThermalAlert_GreaterThan, isGreaterThan);
        if(warning < maxDoubleValue){
            settings.setValue(Setting_ThermalAlert_WarningTemp, warning);
        }
        if(critical < maxDoubleValue){
            settings.setValue(Setting_ThermalAlert_CriticalTemp, critical);
        }

        settings.endArray();
    }
    return success;
}

bool Lights::addGpioInput(QString name, int line, QString edge, bool activeLow)
{
    bool success = activateGpioInput(name, line, edge, activeLow);
    if(success){
        int size = settings.beginReadArray(Setting_GpioInput);
        settings.endArray();

        settings.beginWriteArray(Setting_GpioInput);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_GPIOLineNo, line);
        settings.setValue(Setting_GpioEdge, edge);
        if(activeLow){
            settings.setValue(Setting_GpioActiveLow, activeLow);
        }

        settings.endArray();
    }
    return success;
}

bool Lights::addGpioPWM(QString name, QString outputDev, int rate)
{
    bool success = activateGpioPwm(name, outputDev, rate);
    if(success){
        int size = settings.beginReadArray(Setting_GPIO_PWM);
        settings.endArray();

        settings.beginWriteArray(Setting_GPIO_PWM);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_GPIOOutputDevice, outputDev);
        settings.setValue(Setting_GPIO_PWM_Rate, rate);
        settings.endArray();
    }
    return success;
}

bool Lights::activateGpio(QString name, int line, int initial, int active)
{
    //qDebug() << "init";
    //qDebug() << "name" << name << "line" << line << "initState" << initial << "active" << active;

    if(!name.isEmpty() && line >= 0 && initial >= 0 && active >= 0 && chip != nullptr){
        BasicOnOff::STATE s = static_cast<BasicOnOff::STATE>(initial);
        GpioOutput::VALUE a = static_cast<GpioOutput::VALUE>(active);
        //qDebug() << "name" << name << "line" << QString::number(line);
        gpiod::line gpioLine = chip->get_line(line);
        if(!gpioLine.is_used()){
            shared_ptr<GpioOutput> out = make_shared<GpioOutput>(name, gpioLine, s, a);
            //cout << "Device: " << endl;
            basicIOs->insert(name, out);
            return true;
        }
        else{
            qWarning() << "gpio line in use";
            return false;
        }
    }
    return false;
}

bool Lights::activateThermalSensor(QString name, QString path, int interval, double threshold, double errorValue)
{
    QFile sensor(path);
    if(sensor.open(QIODevice::ReadOnly) && interval < INT_MAX && threshold < std::numeric_limits<double>::max()){
        sensor.close();
        shared_ptr<ThermalSensor> therm = make_shared<SysfsThermalSensor>(name, path, interval, threshold);
        if(errorValue != std::numeric_limits<double>::max()){
            therm->setErrorValue(errorValue);
        }
        therm->readSensor();
        //therm->start();
        thermalSensors->insert(name, therm);
        return true;
    }
    return false;
}

void Lights::activateNwsSensor(QString name, QString station, int interval, double threshold)
{
    shared_ptr<ThermalSensor> therm = make_shared<NWSThermalSensor>(name, station, interval, threshold);
    therm->readSensor();
    thermalSensors->insert(name, therm);
}

bool Lights::activateThermostat(QString name, QString sensor, QString ioDevice, double temperature, QString mode, double threshold, int minOn, int minOff)
{
    shared_ptr<ThermalSensor> Sensor = thermalSensors->value(sensor);
    shared_ptr<BasicOnOff> IODevice = basicIOs->value(ioDevice);
    int modeValue = QMetaEnum::fromType<Thermostat::MODE>().keyToValue(mode.toUpper().toLocal8Bit());
    double maxValue = std::numeric_limits<double>::max();
    if(Sensor != nullptr && IODevice != nullptr && temperature < maxValue && threshold < maxValue && modeValue >=0){
        Thermostat::MODE m = static_cast<Thermostat::MODE>(modeValue);
        shared_ptr<Thermostat> thermostat = make_shared<Thermostat>(name, Sensor, IODevice, temperature, threshold, m);

        if(minOn < std::numeric_limits<int>::max()){
            thermostat->setMinOnTime(minOn);
        }
        if(minOff < std::numeric_limits<int>::max()){
            thermostat->setMinOffTime(minOff);
        }
        thermostats->insert(name, thermostat);
        basicIOs->insert(name, thermostat);
        int success = basicIOs->remove(ioDevice);
        if(success < 1){
            qWarning() << "Failed to remove" << ioDevice;
        }
        connect(thermostat.get(), &Thermostat::onThermostatOn, consoleLogger.get(), &AlertLogger::onThermostatOn);
        connect(thermostat.get(), &Thermostat::onThermostatOff, consoleLogger.get(), &AlertLogger::onThermostatOff);
        return true;
    }

    return false;
}

bool Lights::activateThermalAlert(QString name, QString sensor, double threshold, double warning, double critical, bool isGreaterThan)
{
    double maxDoubleValue = std::numeric_limits<double>::max();
    shared_ptr<ThermalSensor> Sensor = thermalSensors->value(sensor);
    if(Sensor != nullptr && threshold < maxDoubleValue){
        shared_ptr<ThermalAlert> alert = make_shared<ThermalAlert>(name, threshold, isGreaterThan);
        shared_ptr<ActionEvent> event;
        if(warning < maxDoubleValue){
            alert->setWarningTemp(warning);
            event = alert->getWarningEvent();
            events->insert(event->getName(), event);
        }
        if(critical < maxDoubleValue){
            alert->setCriticalTemp(critical);
            event = alert->getCriticalEvent();
            events->insert(event->getName(), event);
        }
        event = alert->getNormalizedEvent();
        events->insert(event->getName(), event);

        event = alert->getAlarmedEvent();
        events->insert(event->getName(), event);

        connect(Sensor.get(), &ThermalSensor::tempChanged, alert.get(), &ThermalAlert::tempChanged);
        connect(alert.get(), &ThermalAlert::onAlert, consoleLogger.get(), &AlertLogger::onThermalAlert);
        Sensor->reportNow();
        thermalAlerts->insert(name, alert);
        return true;
    }

    return false;
}

bool Lights::activateAlarm(QString name, QString time, QString isDayNight)
{
    if(SunRiseSet::validTime(time)){

        shared_ptr<AlarmEvent> evt = make_shared<AlarmEvent>(name, time);
        if(isDayNight.isEmpty()){
            events->insert(name, evt);
            connect(evt.get(), &ActionEvent::onEvent, consoleLogger.get(), &AlertLogger::onActionEvent);
            return true;
        }
        else if(isDayNight.compare("ifDay", Qt::CaseSensitivity::CaseInsensitive) == 0){
            evt->setDayNight(true);
            events->insert(name, evt);
            connect(evt.get(), &ActionEvent::onEvent, consoleLogger.get(), &AlertLogger::onActionEvent);
            connect(evt.get(), &AlarmEvent::isDaylight, sunRiseSet.get(), &SunRiseSet::isDaylight);
            return true;
        }
        else if(isDayNight.compare("ifNight", Qt::CaseSensitivity::CaseInsensitive) == 0){
            evt->setDayNight(false);
            events->insert(name, evt);
            connect(evt.get(), &ActionEvent::onEvent, consoleLogger.get(), &AlertLogger::onActionEvent);
            connect(evt.get(), &AlarmEvent::isDaylight, sunRiseSet.get(), &SunRiseSet::isDaylight);
            return true;
        }
        else {
            return false;
        }

    }

    return false;
}

bool Lights::activateIOGroup(QString name, QStringList devices)
{
    for(const QString &d : devices){
        if(!basicIOs->contains(d)){
            return false;
        }
    }
    shared_ptr<OnOffGroup> grp = make_shared<OnOffGroup>(name);
    for(const QString &d : devices){
        shared_ptr<BasicOnOff> dev = basicIOs->value(d);
        if(dev != nullptr){
            grp->addDevice(dev);
        }
        else{
            return false;
        }
    }
    basicIOs->insert(name, grp);
    return true;
}

bool Lights::activateEventToAction(QString name, QString event, QString state, QString device, bool oneShot)
{
    //qDebug() << "Adding: " << name << "event" << event << "device" << device << "state" << state << "single" << oneShot;
    int stateValue = QMetaEnum::fromType<BasicOnOff::STATE>().keyToValue(state.toUpper().toLocal8Bit());
    if(events->contains(event) && basicIOs->contains(device) && stateValue >= 0){
        BasicOnOff::STATE s = static_cast<BasicOnOff::STATE>(stateValue);
        shared_ptr<BasicOnOff> a = basicIOs->value(device);
        shared_ptr<ActionEvent> e = events->value(event);
        if(a != nullptr && e != nullptr){
            shared_ptr<EventToAction> eta = make_shared<EventToAction>(name, e, s, a, eventActions);
            if(oneShot){
                eta->setOneShot(oneShot);
            }
            eventActions->insert(name, eta);
            return true;
        }
        else{
            return false;
        }
    }

    return false;
}

bool Lights::activateGpioInput(QString name, int line, QString edge, bool activeLow)
{
    int edgeValue = QMetaEnum::fromType<GpioInput::EDGE>().keyToValue(edge.toUpper().toLocal8Bit());
    if(line >= 0 && edgeValue >= 0){
        GpioInput::EDGE e = static_cast<GpioInput::EDGE>(edgeValue);
        gpiod::line gpioLine = chip->get_line(line);
        shared_ptr<GpioInput> input = make_shared<GpioInput>(name, gpioLine, e, activeLow);
        gpioInputs->insert(name, input);
        events->insert(name, input);
        connect(input.get(), &ActionEvent::onEvent, consoleLogger.get(), &AlertLogger::onActionEvent);
        events->insert(name + " Active", input->getActiveEvent());
        connect(input->getActiveEvent().get(), &ActionEvent::onEvent, consoleLogger.get(), &AlertLogger::onActionEvent);
        events->insert(name + " Inactive", input->getInactiveEvent());
        connect(input->getInactiveEvent().get(), &ActionEvent::onEvent, consoleLogger.get(), &AlertLogger::onActionEvent);
        input->emitState();
        return true;
    }
    return false;
}

bool Lights::activateEvapCooler(QString name, QString sensor, QString fan, QString pump, int fanDelay)
{
    shared_ptr<ThermalSensor> Sensor = thermalSensors->value(sensor);
    shared_ptr<BasicOnOff> Fan = basicIOs->value(fan);
    shared_ptr<BasicOnOff> Pump = basicIOs->value(pump);
    if(Sensor != nullptr && Fan != nullptr && Pump != nullptr && fanDelay >= 0 && !name.isEmpty()){
        evapCooler = make_shared<EvapCooler>(name, Sensor, Pump, Fan, fanDelay);
        basicIOs->remove(fan);
        basicIOs->remove(pump);
        basicIOs->insert(name, evapCooler);
        return true;
    }
    return false;
}

bool Lights::activateEvapCoolerFanCondition(QString condition, QString fanName, double temp)
{
    shared_ptr<BasicOnOff> Fan = basicIOs->value(fanName);
    if(evapCooler != nullptr && Fan != nullptr && temp < std::numeric_limits<double>::max()){
        if(condition.compare("med", Qt::CaseSensitivity::CaseInsensitive) == 0){
            evapCooler->setMedCondition(Fan, temp);
            basicIOs->remove(fanName);
            return true;
        }
        else if(condition.compare("high", Qt::CaseSensitivity::CaseInsensitive) == 0){
            evapCooler->setHighCondition(Fan, temp);
            basicIOs->remove(fanName);
            return true;
        }
        else{
            return false;
        }
    }
    return false;
}

bool Lights::activateEvapCoolerPurgePump(QString pump, int purgeTime, int purgeInterval)
{
    shared_ptr<BasicOnOff> Pump = basicIOs->value(pump);
    if(evapCooler != nullptr && Pump != nullptr && purgeTime > 0 && purgeInterval < numeric_limits<int>::max()){
        evapCooler->setPurgePump(Pump, purgeTime, purgeInterval);
        basicIOs->remove(pump);
        return true;
    }
    return false;
}

bool Lights::activateEvapCoolerFanFilter(int filterTime, int timeRemain)
{
    if(evapCooler != nullptr && filterTime >= 0 && timeRemain < numeric_limits<int>::max()){
        evapCooler->setFilterTime(filterTime, timeRemain);
        return true;
    }
    return false;
}

bool Lights::activateEvapCoolerSetMode(QString mode)
{
    if(evapCooler != nullptr ){
        return evapCooler->setMode(mode.toStdString());
    }
    return false;
}

bool Lights::activateGpioPwm(QString name, QString outputDev, int rate)
{
    shared_ptr<BasicOnOff> outDev = basicIOs->value(outputDev);
    if(outDev != nullptr){
        shared_ptr<GpioPWM> pwm = make_shared<GpioPWM>(outDev, name);
        pwm->setRate(rate);
        basicIOs->remove(outputDev);
        basicIOs->insert(name, pwm);
        gpioPWMs->insert(name, pwm);
        return true;
    }
    return false;
}

void Lights::setupThermalSensors()
{
    int size = settings.beginReadArray(Setting_ThermalSensor);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QString path = settings.value(Setting_ThermalPath, "").toString();
        double interval = settings.value(Setting_ThermalInterval, std::numeric_limits<double>::max()).toDouble();
        double threshold = settings.value(Setting_ThermalThreshold, std::numeric_limits<double>::max()).toDouble();
        double errorValue = settings.value(Setting_ThermalErrorValue, std::numeric_limits<double>::max()).toDouble();

        activateThermalSensor(name, path, interval, threshold, errorValue);

    }
    settings.endArray();
}

void Lights::setupSunRiseSet()
{
    double lat = settings.value(Setting_Latitude, 40).toDouble();
    double lon = settings.value(Setting_Longitude, -105).toDouble();
    int timeZone = settings.value(Setting_TimeZone, -7).toInt();
    sunRiseSet->setLatLon(lat, lon, timeZone);
    QString conditionalSunset = settings.value(Setting_SunsetIfBeforeTime, "").toString();
    QString conditionalSunrise = settings.value(Setting_SunriseIfAfterTime, "").toString();
    if(SunRiseSet::validTime(conditionalSunset)){
        sunRiseSet->setConditionalSunSetTime(conditionalSunset);
        connect(sunRiseSet->getConditionalSetAction().get(), &ActionEvent::onEvent, consoleLogger.get(), &AlertLogger::onConditionalSunSet);

    }
    if(SunRiseSet::validTime(conditionalSunrise)){
        sunRiseSet->setConditionalSunRiseTime(conditionalSunrise);
        connect(sunRiseSet->getConditionalRiseAction().get(), &ActionEvent::onEvent, consoleLogger.get(), &AlertLogger::onConditionalSunRise);
    }
}

void Lights::setupNwsThermalSensor()
{
    int size = settings.beginReadArray(Setting_NWS_Key);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QString station = settings.value(Setting_NWS_Station, "").toString();
        double interval = settings.value(Setting_ThermalInterval, std::numeric_limits<double>::max()).toDouble();
        double threshold = settings.value(Setting_ThermalThreshold, std::numeric_limits<double>::max()).toDouble();

        activateNwsSensor(name, station, interval, threshold);

    }
    settings.endArray();
}

void Lights::setupThermostat()
{
    int size = settings.beginReadArray(Setting_Thermostat);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QString sensor = settings.value(Setting_ThermalSensor, "").toString();
        QString ioDevice = settings.value(Setting_GPIOOutputDevice, "").toString();
        QString mode = settings.value(Setting_Thermostat_Mode, "").toString();
        double temperature = settings.value(Setting_Thermostat_Temperature, std::numeric_limits<double>::max()).toDouble();
        double threshold = settings.value(Setting_ThermalThreshold, std::numeric_limits<double>::max()).toDouble();
        int minOn = settings.value(Setting_Thermostat_MinOn, std::numeric_limits<int>::max()).toInt();
        int minOff = settings.value(Setting_Thermostat_MinOff, std::numeric_limits<int>::max()).toInt();

        activateThermostat(name, sensor, ioDevice, temperature, mode, threshold, minOn, minOff);

    }
    settings.endArray();
}

void Lights::setupThermalAlerts()
{
    int size = settings.beginReadArray(Setting_ThermalAlert);
    double maxDoubleValue = std::numeric_limits<double>::max();
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QString sensor = settings.value(Setting_ThermalSensor, "").toString();
        double threshold = settings.value(Setting_ThermalThreshold, maxDoubleValue).toDouble();
        double warning = settings.value(Setting_ThermalAlert_WarningTemp, maxDoubleValue).toDouble();
        double critical = settings.value(Setting_ThermalAlert_CriticalTemp, maxDoubleValue).toDouble();
        bool isGreaterThan = settings.value(Setting_ThermalAlert_GreaterThan, false).toBool();

        activateThermalAlert(name, sensor, threshold, warning, critical, isGreaterThan);

    }
    settings.endArray();
}

void Lights::setupEvents()
{
    events->insert(Setting_Sunset, sunRiseSet->getSetAction());
    events->insert(Setting_Sunrise, sunRiseSet->getRiseAction());
    QString conditionalSunset = settings.value(Setting_SunsetIfBeforeTime, "").toString();
    QString conditionalSunrise = settings.value(Setting_SunriseIfAfterTime, "").toString();
    if(SunRiseSet::validTime(conditionalSunset)){
        events->insert(Setting_SunsetIfBeforeTime, sunRiseSet->getConditionalSetAction());
    }
    if(SunRiseSet::validTime(conditionalSunrise)){
        events->insert(Setting_SunriseIfAfterTime, sunRiseSet->getConditionalRiseAction());
    }
    int size = settings.beginReadArray(Setting_Alarm);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QString time = settings.value(Setting_AlarmTime, "").toString();
        QString isDayNight = settings.value(Setting_AlarmIsDay, "").toString();

        activateAlarm(name, time, isDayNight);

    }
    settings.endArray();

}

void Lights::setupIOGroups()
{
    int size = settings.beginReadArray(Setting_IOGroup);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QStringList devices = settings.value(Setting_IOGroupList, "").toStringList();

        activateIOGroup(name, devices);

    }
    settings.endArray();
}

void Lights::setupEventToAction()
{
    int size = settings.beginReadArray(Setting_EventToAction);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QString event = settings.value(Setting_EventToAction_Event, "").toString();
        QString state = settings.value(Setting_EventToAction_State, "").toString();
        QString device = settings.value(Setting_EventToAction_Action, "").toString();
        bool oneShot = settings.value(Setting_EventToAction_OneShot, "").toBool();

        activateEventToAction(name, event, state, device, oneShot);

    }
    settings.endArray();
}

void Lights::setupGpioInput()
{
    int size = settings.beginReadArray(Setting_GpioInput);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        int line = settings.value(Setting_GPIOLineNo, -1).toInt();
        QString edge = settings.value(Setting_GpioEdge, "").toString();
        bool activeLow = settings.value(Setting_GpioActiveLow, false).toBool();
        activateGpioInput(name, line, edge, activeLow);

    }
    settings.endArray();
}

void Lights::setupEvapCooler()
{
    settings.beginGroup(Setting_EvapCooler);
    bool enabled = settings.value(Setting_EvapCooler, false).toBool();
    if(enabled){

        QString name = settings.value(Setting_Name, "").toString();
        QString sensor = settings.value(Setting_ThermalSensor, "").toString();
        QString pump = settings.value(Setting_EvapCooler_Pump, "").toString();
        QString fanLow = settings.value(Setting_EvapCooler_FanLow, "").toString();
        int fanDelay = settings.value(Setting_EvapCooler_FanDelay, -1).toInt();
        bool success = activateEvapCooler(name, sensor, fanLow, pump, fanDelay);
        if(success){
            connect(evapCooler.get(), &EvapCooler::statusMessage, consoleLogger.get(), &AlertLogger::onGeneric);
            // Fan Med
            settings.beginGroup(Setting_EvapCooler_FanMed);
            if(settings.value(Setting_EvapCooler_FanMed, false).toBool()){
                QString fanName = settings.value(Setting_Name, "").toString();
                double temp = settings.value(Setting_Thermostat_Temperature, 0.0).toDouble();
                activateEvapCoolerFanCondition("med", fanName, temp);
            }
            settings.endGroup();

            //Fan High
            settings.beginGroup(Setting_EvapCooler_FanHigh);
            if(settings.value(Setting_EvapCooler_FanHigh, false).toBool()){
                QString fanName = settings.value(Setting_Name, "").toString();
                double temp = settings.value(Setting_Thermostat_Temperature, 0.0).toDouble();
                activateEvapCoolerFanCondition("high", fanName, temp);
            }
            settings.endGroup();

            //Purge
            settings.beginGroup(Setting_EvapCooler_Purge);
            if(settings.value(Setting_EvapCooler_Purge, false).toBool()){
                QString purgePump = settings.value(Setting_EvapCooler_Pump, "").toString();
                int time = settings.value(Setting_EvapCooler_PurgTime, 0).toInt();
                int interval = settings.value(Setting_EvapCooler_PurgeInterval, numeric_limits<int>::max()).toInt();
                activateEvapCoolerPurgePump(purgePump, time, interval);
            }
            settings.endGroup();

            //Filter
            settings.beginGroup(Setting_EvapCooler_Filter);
            if(settings.value(Setting_EvapCooler_Filter, false).toBool()){
                int filterTime = settings.value(Setting_EvapCooler_FilterTime, -1).toInt();
                int filterLife = settings.value(Setting_EvapCooler_FilterLife, numeric_limits<int>::max()).toInt();
                activateEvapCoolerFanFilter(filterTime, filterLife);
            }
            settings.endGroup();

            //Mode
            QString mode = settings.value(Setting_EvapCooler_Mode, "").toString();
            if(!mode.isEmpty()){
                activateEvapCoolerSetMode(mode);
            }
        }

    }
    settings.endGroup();

}

void Lights::setupPWMs()
{
    int size = settings.beginReadArray(Setting_GPIO_PWM);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        int rate = settings.value(Setting_GPIO_PWM_Rate, -1).toInt();
        QString outputPin = settings.value(Setting_GPIOOutputDevice, "").toString();
        if(!name.isEmpty() && rate >= 0 && rate <= 100 && !outputPin.isEmpty()){
            activateGpioPwm(name, outputPin, rate);
        }
    }
    settings.endArray();
}

bool Lights::updateLocation(QVariant location)
{
    QVariantList list = location.toList();
    if(list.size() == 2){
        bool isLat, isLon;
        double lat = list.at(0).toDouble(&isLat);
        double lon = list.at(1).toDouble(&isLon);
        if(isLat && isLon){
            if(lat >= -90 && lat <= 90 && lon >= -180 && lon <= 180){
                settings.setValue(Setting_Latitude, lat);
                settings.setValue(Setting_Longitude, lon);
                setupSunRiseSet();
            }
            return true;
        }
    }
    return false;
}

bool Lights::setOption(QString name, QVariant value)
{
    if(name.compare("Location", Qt::CaseInsensitive) == 0){
        return updateLocation(value);
    }
    return false;
}

QJsonDocument Lights::lsOptions()
{
    QJsonArray ret;
    QVariantMap map;

    map.insert(keyName, "Location");
    map.insert(keyValueType, "List<double>");
    map.insert(keyDesc, "Sets longitude and latitude for sunrise & sunset events");
    ret.append(QJsonObject::fromVariantMap(map));

    return QJsonDocument(ret);
}

string Lights::lsBasicOnOff()
{
    return lsStatusable(basicIOs);
}

string Lights::lsGpioConfig()
{
    string ret = "";
    int size = settings.beginReadArray(Setting_GPIOOutputDevice);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QString line = settings.value(Setting_GPIOLineNo, -1).toString();
        QString initial = settings.value(Setting_GPIOInitialState).toString();
        QString active = settings.value(Setting_GPIOActiveState).toString();

        ret += "\tName: " + name.toStdString() + "\n";
        ret += "\t\tGPIO Line: " + line.toStdString() + "\n";
        ret += "\t\tInitial State: " + initial.toStdString() + "\n";
        ret += "\t\tActive: " + active.toStdString() + "\n";

    }
    settings.endArray();

    return ret;
}

string Lights::lsThermalSensor()
{
    return lsStatusable(thermalSensors);
}

string Lights::lsThermalSensorConfig()
{
    string ret = "";

    int size = settings.beginReadArray(Setting_ThermalSensor);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        QString path = settings.value(Setting_ThermalPath, "").toString();
        QString interval = settings.value(Setting_ThermalInterval, std::numeric_limits<double>::max()).toString();
        QString threshold = settings.value(Setting_ThermalThreshold, std::numeric_limits<double>::max()).toString();

        ret += "\tName: " + name.toStdString() + "\n";
        ret += "\t\tPath: " + path.toStdString() + "\n";
        ret += "\t\tScan Interval: " + interval.toStdString() + "\n";
        ret += "\t\tReporting Threshold: " + threshold.toStdString() + "\n";

    }
    settings.endArray();

    return ret;
}

string Lights::lsSunRiseSet()
{
    return sunRiseSet->getStatus().toStdString();
}

string Lights::lsThermalAlerts()
{
    return lsStatusable(thermalAlerts);
}

string Lights::lsEvents()
{
    return lsStatusable(events);
}

string Lights::lsEventActions()
{
    return lsStatusable(eventActions);
}

string Lights::lsGpioInput()
{
    return lsStatusable(gpioInputs);
}

string Lights::lsEvapCooler()
{
    if(evapCooler != nullptr){
        return evapCooler->getStatus().toStdString();
    }
    else{
        return "disabled\n";
    }
}

string Lights::lsEvapCoolerModes()
{
    if(evapCooler != nullptr){
        return evapCooler->lsModes();
    }
    else{
        return "disabled\n";
    }
}

QJsonDocument Lights::lsDeviceOptions()
{
    QVariantMap ret;
    ret.insert("Global", lsOptions().array());

    QStringList keys = basicIOs->keys();
    foreach(QString key, keys){
        ret.insert(key, basicIOs->value(key)->lsOptions().array());
    }
    return QJsonDocument::fromVariant(ret);
}

bool Lights::setDeviceOption(QString device, QString option, QVariant value)
{
    if(device.compare("Global", Qt::CaseInsensitive) == 0){
        return setOption(option, value);
    }
    else{
        QStringList keys = basicIOs->keys();
        foreach(QString key, keys){
            if(key.compare(device, Qt::CaseInsensitive) == 0){
                return basicIOs->value(key)->setOption(option, value);
            }
        }
    }
    qInfo() << "device not found";
    return false;
}

bool Lights::addAlarm(QString name, QString time, QString isDayNight)
{
    bool success = activateAlarm(name, time, isDayNight);
    if(success){
        int size = settings.beginReadArray(Setting_Alarm);
        settings.endArray();

        settings.beginWriteArray(Setting_Alarm);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_AlarmTime, time);
        if(!isDayNight.isEmpty()){
            settings.setValue(Setting_AlarmIsDay, isDayNight);
        }
        settings.endArray();
    }
    return success;
}

bool Lights::addOnOffGroup(QString name, QStringList devices)
{
    bool success = activateIOGroup(name, devices);
    if(success){
        int size = settings.beginReadArray(Setting_IOGroup);
        settings.endArray();

        settings.beginWriteArray(Setting_IOGroup);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_IOGroupList, devices);
        settings.endArray();
    }
    return success;
}

bool Lights::addEventAction(QString name, QString event, QString device, QString state, bool oneShot)
{
    bool success = activateEventToAction(name, event, state, device, oneShot);
    if(success && !oneShot){
        int size = settings.beginReadArray(Setting_EventToAction);
        settings.endArray();

        settings.beginWriteArray(Setting_EventToAction);
        settings.setArrayIndex(size);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_EventToAction_Event, event);
        settings.setValue(Setting_EventToAction_Action, device);
        settings.setValue(Setting_EventToAction_State, state);
        settings.endArray();
    }
    return success;
}

template<class T>
string Lights::lsStatusable(shared_ptr<QHash<QString, shared_ptr<T> > > hash)
{
    QString ret;
    if(hash->size() > 0){
        QStringList keys = hash->keys();
        foreach(QString key, keys){
            auto value = hash->value(key);
            shared_ptr<Statusable> status = dynamic_pointer_cast<T>(value);
            ret += status->getStatus();
        }

    }
    return ret.trimmed().toStdString();
}

bool Lights::setRelay(QString name, QString state)
{
    if(state.compare("ON", Qt::CaseSensitivity::CaseInsensitive) == 0){
        return turnOn(name);
    }
    else if(state.compare("OFF", Qt::CaseSensitivity::CaseInsensitive) == 0){
        return turnOff(name);
    }
    else if(state.compare("AUTO", Qt::CaseSensitivity::CaseInsensitive) == 0){
        return turnAuto(name);

    }
    return false;
}

bool Lights::enableEvapCooler(bool enabled)
{
    settings.beginGroup(Setting_EvapCooler);
    settings.setValue(Setting_EvapCooler, enabled);
    settings.endGroup();
    return true;
}

bool Lights::setGpioChip(QString chipName)
{
    settings.setValue(Setting_GPIO_Chip, chipName);
    qInfo() << "Restart the program to use the new chip";
    return true;
}

bool Lights::configEvapCooler(QString name, QString sensor, QString pump, QString fan, int fanDelay)
{
    bool success = activateEvapCooler(name, sensor, fan, pump, fanDelay);
    if(success){
        settings.beginGroup(Setting_EvapCooler);
        settings.setValue(Setting_Name, name);
        settings.setValue(Setting_ThermalSensor, sensor);
        settings.setValue(Setting_EvapCooler_Pump, pump);
        settings.setValue(Setting_EvapCooler_FanLow, fan);
        settings.setValue(Setting_EvapCooler_FanDelay, fanDelay);
        settings.endGroup();
    }
    return success;
}

bool Lights::configEvapCoolerFanCondition(QString condition, QString fanName, double temp)
{
    bool success = activateEvapCoolerFanCondition(condition, fanName, temp);
    if(success){
        settings.beginGroup(Setting_EvapCooler);
        if(condition.compare("med", Qt::CaseSensitivity::CaseInsensitive) == 0){
            settings.beginGroup(Setting_EvapCooler_FanMed);

            settings.setValue(Setting_EvapCooler_FanMed, true);
            settings.setValue(Setting_Name, fanName);
            settings.setValue(Setting_Thermostat_Temperature, temp);

            settings.endGroup();

        }
        else if(condition.compare("high", Qt::CaseSensitivity::CaseInsensitive) == 0){
            settings.beginGroup(Setting_EvapCooler_FanHigh);

            settings.setValue(Setting_EvapCooler_FanHigh, true);
            settings.setValue(Setting_Name, fanName);
            settings.setValue(Setting_Thermostat_Temperature, temp);

            settings.endGroup();


        }
        settings.endGroup();
    }
    return success;
}

bool Lights::configEvapCoolerPurgePump(QString pump, int purgeTime, int purgeInterval)
{
    bool success = activateEvapCoolerPurgePump(pump, purgeTime, purgeInterval);
    if(success){
        settings.beginGroup(Setting_EvapCooler);
        settings.beginGroup(Setting_EvapCooler_Purge);

        settings.setValue(Setting_EvapCooler_Purge, true);
        settings.setValue(Setting_EvapCooler_Pump, pump);
        settings.setValue(Setting_EvapCooler_PurgTime, purgeTime);
        settings.setValue(Setting_EvapCooler_PurgeInterval, purgeInterval);

        settings.endGroup();
        settings.endGroup();
    }
    return success;
}

bool Lights::configEvapCoolerFanFilter(int filterTime, int timeRemain)
{
    bool success = activateEvapCoolerFanFilter(filterTime, timeRemain);
    if(success){
        settings.beginGroup(Setting_EvapCooler);
        settings.beginGroup(Setting_EvapCooler_Filter);
        settings.setValue(Setting_EvapCooler_FilterLife, timeRemain);
        settings.setValue(Setting_EvapCooler_FilterTime, filterTime);
        settings.endGroup();
        settings.endGroup();
    }
    return success;
}

bool Lights::configEvapCoolerSetMode(QString mode)
{
    bool success = activateEvapCoolerSetMode(mode);
    if(success){
        settings.beginGroup(Setting_EvapCooler);
        settings.setValue(Setting_EvapCooler_Mode, mode);
        settings.endGroup();
    }
    return success;
}

void Lights::setupGpios()
{
    //qDebug() << "init";
    int size = settings.beginReadArray(Setting_GPIOOutputDevice);
    for (int i = 0; i < size ;i++ ) {
        settings.setArrayIndex(i);
        QString name = settings.value(Setting_Name, "").toString();
        int line = settings.value(Setting_GPIOLineNo, -1).toInt();
        //qDebug() << "name" << name << "line" << QString::number(line);
        int initial = QMetaEnum::fromType<BasicOnOff::STATE>().keyToValue(settings.value(Setting_GPIOInitialState).toString().toLocal8Bit());
        int active = QMetaEnum::fromType<GpioOutput::VALUE>().keyToValue(settings.value(Setting_GPIOActiveState).toString().toLocal8Bit());

        activateGpio(name, line, initial, active);

    }
    settings.endArray();
}
