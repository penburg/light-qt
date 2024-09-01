#include "basiconoff.h"

BasicOnOff::BasicOnOff(QString n)
{
    name = n;
}

void BasicOnOff::setAuto()
{
    qInfo() << "Not Implemented";
}

const QString &BasicOnOff::getName() const
{
    return name;
}

bool BasicOnOff::setOption(QString name, QVariant value)
{
    if(name.compare("State", Qt::CaseInsensitive) == 0){
        bool valid = false;
        QMetaEnum allStates = QMetaEnum::fromType<BasicOnOff::STATE>();
        STATE s = static_cast<STATE>(allStates.keyToValue(value.toString().toLocal8Bit(), &valid));
        if(valid){
            switch(s){
            case ON:
                turnOn();
                break;
            case OFF:
                turnOff();
                break;
            case AUTO:
                setAuto();
                break;
            }
        }
        return valid;
    }
    return false;
}

QJsonDocument BasicOnOff::lsOptions()
{
    QJsonArray ret;
    QVariantMap map;

    map.insert(keyName, "State");
    map.insert(keyValueType, "String");
    map.insert(keyDesc, "Sets device to ON, OFF, or AUTO");
    ret.append(QJsonObject::fromVariantMap(map));

    return QJsonDocument(ret);
}

QJsonDocument BasicOnOff::jsonStatus() const
{
    QJsonArray ret;
    QVariantMap map;

    map.insert(sKeyName, "Name");
    map.insert(sKeyValue, name);
    map.insert(sKeyDesc, "The device name");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "State");
    map.insert(sKeyValue, state);
    map.insert(sKeyDesc, "Curent state of the on / off device");
    ret.append(QJsonObject::fromVariantMap(map));

    return QJsonDocument(ret);
}

QString BasicOnOff::getStatus() const
{
    QString ret;
    ret += "Name: " + name + "\n";
    ret += "\tState: " + QString(QMetaEnum::fromType<BasicOnOff::STATE>().valueToKey(state)) + "\n";
    return ret;
}

BasicOnOff::STATE BasicOnOff::getState() const
{
    return state;
}
