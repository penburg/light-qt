#include "eventtoaction.h"

EventToAction::EventToAction(QString n, shared_ptr<ActionEvent> e, BasicOnOff::STATE s, shared_ptr<BasicOnOff> a, shared_ptr<QHash<QString, shared_ptr<EventToAction>>> p, QObject *pnt)
    : QObject{pnt}
{
    name = n;
    event = e;
    state = s;
    action = a;
    parent = p;
    oneShot = false;
    connect(event.get(), &ActionEvent::onEvent, this, &EventToAction::onAction);
}

void EventToAction::setOneShot(bool shot)
{
    oneShot = shot;
}

QJsonDocument EventToAction::jsonStatus() const
{
    QJsonArray ret;
    QVariantMap map;

    map.insert(sKeyName, "Name");
    map.insert(sKeyValue, name);
    map.insert(sKeyDesc, "The sensor name");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Trigger");
    map.insert(sKeyValue, event->getName());
    map.insert(sKeyDesc, "The event that triggers the action");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Device");
    map.insert(sKeyValue, action->getName());
    map.insert(sKeyDesc, "The device that is acted upon");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Set to");
    map.insert(sKeyValue, QString(QMetaEnum::fromType<BasicOnOff::STATE>().valueToKey(state)));
    map.insert(sKeyDesc, "What the device will be set to");
    ret.append(QJsonObject::fromVariantMap(map));

    return QJsonDocument(ret);
}

QString EventToAction::getStatus() const
{
    QString ret = "Name: " + name + "\n";
    ret += "\tTrigger: " + event->getName() + "\n";
    ret += "\tDevice: " + action->getName() + "\n";
    ret += "\tSet to: " + QString(QMetaEnum::fromType<BasicOnOff::STATE>().valueToKey(state)) + "\n";


    return ret;
}

void EventToAction::onAction(QString eventName)
{
    switch (state){
    case BasicOnOff::ON:
        action->turnOn();
        break;
    case BasicOnOff::OFF:
        action->turnOff();
        break;
    case BasicOnOff::AUTO:
        action->setAuto();
        break;
    }

    QString msg  = name + " complete: "  + eventName + " set " + action->getName() + " to " + QMetaEnum::fromType<BasicOnOff::STATE>().valueToKey(state);
    emit onComplete(msg);
    if(oneShot){
        parent->remove(name);
        disconnect(event.get(), &ActionEvent::onEvent, this, &EventToAction::onAction);
    }
}
