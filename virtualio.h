#ifndef VIRTUALIO_H
#define VIRTUALIO_H

#include "basiconoff.h"
#include <QObject>

class VirtualIO : public BasicOnOff
{
public:
    VirtualIO(QString name);

    // BasicOnOff interface
public slots:
    void turnOn();
    void turnOff();
};

#endif // VIRTUALIO_H
