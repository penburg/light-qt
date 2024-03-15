#include "virtualio.h"

VirtualIO::VirtualIO(QString name) : BasicOnOff(name)
{

}

void VirtualIO::turnOn()
{
    qInfo() << "virtual function";
}

void VirtualIO::turnOff()
{
    qInfo() << "virtual function";
}
