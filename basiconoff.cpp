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
