#include "thermalalert.h"

ThermalAlert::ThermalAlert(QString n, double threshold, bool greater, QObject *parent) : QObject(parent)
{
    this->name = n;
    this->activeAlert = AlertType::Thermal::None;
    this->thresholdTemp = threshold;
    this->greaterThan = greater;
    this->warningTemp = std::numeric_limits<double>::min();
    this->criticalTemp = std::numeric_limits<double>::min();
    this->lastTemp = greater ? std::numeric_limits<double>::max() : std::numeric_limits<double>::min();
    QString hl = greaterThan ? "High" : "Low";
    alarmedEvent = make_shared<ActionEvent>(name + "_" + hl);
    normalizedEvent = make_shared<ActionEvent>(name + "_Normalized");
}


void ThermalAlert::setWarningTemp(double newWarningTemp)
{
    warningTemp = newWarningTemp;
    warningEvent = make_shared<ActionEvent>(name + "_Warning");
}

void ThermalAlert::setCriticalTemp(double newCriticalTemp)
{
    criticalTemp = newCriticalTemp;
    criticalEvent = make_shared<ActionEvent>(name + "_Critical");
}

QString ThermalAlert::getStatus() const
{
    //qDebug() << "Alert:" << activeAlert << QMetaEnum::fromType<AlertType::Thermal>().valueToKey(activeAlert) << QVariant::fromValue(activeAlert).toString();
    QString ret;
    ret += "Name:\t" + name + "\n";
    ret += "\tAlert: ";
    ret += QMetaEnum::fromType<AlertType::Thermal>().valueToKey(activeAlert);
    ret +=+ "\n";
    ret += "\tLast Temp: " + ThermalSensor::getFormatedTemp(lastTemp) + "\n";
    ret += "\tMode: ";
    ret += greaterThan ? "Greater Than" : "Less Than";
    ret += "\n";
    ret += "\tAlert Temp:\t" + ThermalSensor::getFormatedTemp(thresholdTemp) + "\n";
    ret += "\tWarning Temp:\t" + ThermalSensor::getFormatedTemp(warningTemp) + "\n";
    ret += "\tCritical Temp:\t" + ThermalSensor::getFormatedTemp(criticalTemp) + "\n";
    return ret;
}

QJsonDocument ThermalAlert::jsonStatus() const
{
    QJsonArray ret;
    QVariantMap map;

    map.insert(sKeyName, "Name");
    map.insert(sKeyValue, name);
    map.insert(sKeyDesc, "The alert name");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Last Temp");
    map.insert(sKeyValue, ThermalSensor::getFormatedTemp(lastTemp));
    map.insert(sKeyDesc, "The latest temperatue received");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Alert Status");
    map.insert(sKeyValue, QMetaEnum::fromType<AlertType::Thermal>().valueToKey(activeAlert));
    map.insert(sKeyDesc, "Current status of the alert");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Mode");
    map.insert(sKeyValue, greaterThan ? "Greater Than" : "Less Than");
    map.insert(sKeyDesc, "Which direction the alert follows");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Alert Temp");
    map.insert(sKeyValue, ThermalSensor::getFormatedTemp(thresholdTemp));
    map.insert(sKeyDesc, "The threshold to activate the alert");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Warning Temp");
    map.insert(sKeyValue, ThermalSensor::getFormatedTemp(warningTemp));
    map.insert(sKeyDesc, "The threshold to activate the warning alert");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Critical Temp");
    map.insert(sKeyValue, ThermalSensor::getFormatedTemp(warningTemp));
    map.insert(sKeyDesc, "The threshold to activate the critical alert");
    ret.append(QJsonObject::fromVariantMap(map));

    return QJsonDocument(ret);
}

void ThermalAlert::tempChanged(double temp)
{
    AlertType::Thermal alert = AlertType::Thermal::None;
    if(this->greaterThan){
        if(alert == AlertType::Thermal::None && this->criticalTemp != std::numeric_limits<double>::min()){
            alert = checkTemp(temp, this->criticalTemp, AlertType::Thermal::Critical);
        }
        if(alert == AlertType::Thermal::None && this->warningTemp != std::numeric_limits<double>::min()){
            alert = checkTemp(temp, this->warningTemp, AlertType::Thermal::Warning);
        }
        if(alert == AlertType::Thermal::None){
            alert = checkTemp(temp, this->thresholdTemp, AlertType::Thermal::High);
        }
    }
    else{
        if(alert == AlertType::Thermal::None && this->criticalTemp != std::numeric_limits<double>::min()){
            alert = checkTemp(this->criticalTemp, temp, AlertType::Thermal::Critical);
        }
        if(alert == AlertType::Thermal::None && this->warningTemp != std::numeric_limits<double>::min()){
            alert = checkTemp(this->warningTemp, temp, AlertType::Thermal::Warning);
        }
        if(alert == AlertType::Thermal::None){
            alert = checkTemp(this->thresholdTemp, temp, AlertType::Thermal::Low);
        }

    }

    if(alert != AlertType::Thermal::None && alert != activeAlert){
        QString msg = this->name;
        msg += " temperature ";
        msg += QMetaEnum::fromType<AlertType::Thermal>().valueToKey(alert);
        msg += " (";
        msg += ThermalSensor::getFormatedTemp(temp);
        msg += ")";
        emit onAlert(msg, alert);
        sendActionEvent(alert);
        activeAlert = alert;
    }
    else if(alert == AlertType::Thermal::None && activeAlert != AlertType::Thermal::None){
        QString msg = this->name;
        msg += " temperature ";
        msg += QMetaEnum::fromType<AlertType::Thermal>().valueToKey(AlertType::Thermal::Normalized);
        msg += " (";
        msg += ThermalSensor::getFormatedTemp(temp);
        msg += ")";
        emit onAlert(msg, alert);
        sendActionEvent(alert);
        activeAlert = AlertType::Thermal::None;
    }

    lastTemp = temp;
}

const shared_ptr<ActionEvent> &ThermalAlert::getNormalizedEvent() const
{
    return normalizedEvent;
}

const shared_ptr<ActionEvent> &ThermalAlert::getCriticalEvent() const
{
    return criticalEvent;
}

const shared_ptr<ActionEvent> &ThermalAlert::getWarningEvent() const
{
    return warningEvent;
}

const shared_ptr<ActionEvent> &ThermalAlert::getAlarmedEvent() const
{
    return alarmedEvent;
}


AlertType::Thermal ThermalAlert::checkTemp(double left, double right, AlertType::Thermal ret)
{
    if(left > right){
        return ret;
    }
    else{
        return AlertType::Thermal::None;
    }
}

void ThermalAlert::sendActionEvent(AlertType::Thermal type)
{
    switch(type){
    case AlertType::Thermal::Normalized:
        normalizedEvent->emitEvent();
        break;
    case AlertType::Thermal::Low:
        alarmedEvent->emitEvent();
        break;
    case AlertType::Thermal::High:
        alarmedEvent->emitEvent();
        break;
    case AlertType::Thermal::Warning:
        warningEvent->emitEvent();
        break;
    case AlertType::Thermal::Critical:
        criticalEvent->emitEvent();
        break;
    case AlertType::Thermal::None:
        break;
    }
}
