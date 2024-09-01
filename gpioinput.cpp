#include "gpioinput.h"

GpioInput::GpioInput(QString n, gpiod::line l, EDGE e, bool activeLow) : ActionEvent(n)
{
    thread = make_shared<GpioInputThread>(n, l, static_cast<GpioInputThread::EDGE>(e), activeLow);
    connect(thread.get(), &GpioInputThread::onEvent, this, &GpioInput::transferEvent);

    activeEvent = make_shared<ActionEvent>(getName() + " Active");
    inactiveEvent = make_shared<ActionEvent>(getName() + " Inactive");
    connect(thread.get(), &GpioInputThread::onActive, activeEvent.get(), &ActionEvent::emitEvent);
    connect(thread.get(), &GpioInputThread::onInactive, inactiveEvent.get(), &ActionEvent::emitEvent);


}

GpioInput::~GpioInput()
{
    if(thread->isRunning()){
        thread->wait(2000);
        if(thread->isRunning()){
            thread->requestInterruption();
        }
    }
}



void GpioInput::shutdown()
{
    thread->shutdown();
}

QString GpioInput::getStatus() const
{
    QString ret = ActionEvent::getStatus();
    ret += thread->getStatus();
    return ret;
}

void GpioInput::transferEvent(QString msg)
{
    emit onEvent(msg);
}

const shared_ptr<ActionEvent> &GpioInput::getInactiveEvent() const
{
    return inactiveEvent;
}

void GpioInput::emitState()
{
    thread->triggerState();
}

QJsonDocument GpioInput::jsonStatus() const
{
    QJsonArray ret = ActionEvent::jsonStatus().array();
    QVariantMap map;

    map.insert(sKeyName, "ThreadStatus");
    map.insert(sKeyValue, thread->jsonStatus());
    map.insert(sKeyDesc, "The input thread status");
    ret.append(QJsonObject::fromVariantMap(map));


    return QJsonDocument(ret);
}

const shared_ptr<ActionEvent> &GpioInput::getActiveEvent() const
{
    return activeEvent;
}
