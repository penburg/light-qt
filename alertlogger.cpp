#include "alertlogger.h"

AlertLogger::AlertLogger(std::ostream *o, QObject *parent) : QObject(parent)
{
    out = o;
}

void AlertLogger::onThermalAlert(QString msg, AlertType::Thermal type)
{
    Q_UNUSED(type);
    *out << getTimestamp() << " - " << msg.toStdString() << endl;
}

void AlertLogger::onSunRise()
{
    *out << getTimestamp() << " - Sun Rise" << endl;
}

void AlertLogger::onSunSet()
{

    *out << getTimestamp() << " - Sun Set" <<  endl;
}

void AlertLogger::onConditionalSunRise()
{
    *out << getTimestamp() << " - Conditional Sun Rise" << endl;
}

void AlertLogger::onConditionalSunSet()
{
    *out << getTimestamp() << " - Conditional Sun Set" <<  endl;
}

void AlertLogger::onThermostatOn(QString name)
{
    *out << getTimestamp() << " - " << name.toStdString() << " Thermostat On" <<  endl;
}

void AlertLogger::onThermostatOff(QString name)
{
    *out << getTimestamp() << " - " << name.toStdString() << " Thermostat Off" <<  endl;
}

void AlertLogger::onActionEvent(QString name)
{
    *out << getTimestamp() << " - Event " << name.toStdString() <<  endl;
}

void AlertLogger::onGeneric(QString msg)
{
    *out << getTimestamp() << " - " << msg.toStdString() <<  endl;
}

string AlertLogger::getTimestamp()
{
    QString format = "yyyy-MM-dd hh:mm:ss";
    QDateTime dateObj = QDateTime::currentDateTime();
    return dateObj.toString(format).toStdString();
}
