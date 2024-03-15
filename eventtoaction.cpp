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
