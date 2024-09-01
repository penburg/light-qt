#include "gpiooutput.h"

GpioOutput::GpioOutput(QString n, gpiod::line l, STATE init, VALUE on) : BasicOnOff(n)
{
    line = unique_ptr<gpiod::line>(new gpiod::line(l));
    state = init;
    int flags = on == VALUE::HIGH ? 0 : gpiod::line_request::FLAG_ACTIVE_LOW.to_ulong();


    int newValue = init == STATE::ON ? VALUE::HIGH : VALUE::LOW;
    line->request({n.toStdString(), gpiod::line_request::DIRECTION_OUTPUT, flags },  newValue );
    line->set_value(newValue);
}

GpioOutput::~GpioOutput()
{
    line->release();
}

void GpioOutput::varDump()
{
    cout << *this;
}

ostream& operator<<(ostream& os, const GpioOutput& obj){

    os << "Name: " << obj.getName().toStdString() << endl;
    os << "State: " << obj.getState() << " (" << QMetaEnum::fromType<BasicOnOff::STATE>().valueToKey(obj.getState()) << ")" << endl;
    os << "Value: " << obj.getCurrentValue() << endl;
    return os;
}

void GpioOutput::turnOn()
{
    line->set_value(VALUE::HIGH);
    state = STATE::ON;
}

void GpioOutput::turnOff()
{
    line->set_value(VALUE::LOW);
    state = STATE::OFF;
}

QJsonDocument GpioOutput::jsonStatus() const
{
    QJsonArray ret = BasicOnOff::jsonStatus().array();
    QVariantMap map;
    map.clear();
    map.insert(sKeyName, "Active");
    map.insert(sKeyValue, QString(QMetaEnum::fromType<GpioOutput::VALUE>().valueToKey(line->get_value())));
    map.insert(sKeyDesc, "The active state of the output device");
    ret.append(QJsonObject::fromVariantMap(map));

    return QJsonDocument(ret);
}

QString GpioOutput::getStatus() const
{
    QString ret = BasicOnOff::getStatus();;
    ret += "\tActive: " + QString(QMetaEnum::fromType<GpioOutput::VALUE>().valueToKey(line->get_value())) + "\n";
    return ret;
}


int GpioOutput::getCurrentValue() const
{
    return line->get_value();
}


const QString &GpioOutput::getName() const
{
    return name;
}
