#ifndef GPIOINPUT_H
#define GPIOINPUT_H

#include "actionevent.h"
#include <QObject>
#include <QDebug>
#include <QMetaEnum>
#include <gpiod.hpp>
#include <memory>
#include "gpioinputthread.h"

using namespace std;

class GpioInput : public ActionEvent
{
    Q_OBJECT

public:
    enum EDGE{
        RISING,
        FALLING,
        BOTH
    };
    Q_ENUM(EDGE)

    GpioInput(QString n, gpiod::line l, EDGE e, bool activeLow = false);
    ~GpioInput();
    void shutdown();
    QString getStatus() const override;
    const shared_ptr<ActionEvent> &getActiveEvent() const;
    const shared_ptr<ActionEvent> &getInactiveEvent() const;
    void emitState();

private:
    void transferEvent(QString msg);
    shared_ptr<GpioInputThread> thread;
    shared_ptr<ActionEvent> activeEvent;
    shared_ptr<ActionEvent> inactiveEvent;

};

#endif // GPIOINPUT_H
