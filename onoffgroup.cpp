#include "onoffgroup.h"

OnOffGroup::OnOffGroup(QString name): BasicOnOff(name)
{

}

QString OnOffGroup::getStatus() const
{
    QString ret = BasicOnOff::getStatus();
    for(const shared_ptr<BasicOnOff> &d : devices){
        ret += "\t" + d->getStatus().replace("\t", "\t\t");
    }

    return ret;
}

void OnOffGroup::addDevice(shared_ptr<BasicOnOff> d)
{
    devices.push_back(d);
}

void OnOffGroup::turnOn()
{
    for(const shared_ptr<BasicOnOff> &d : devices){
        d->turnOn();
    }
    state = BasicOnOff::STATE::ON;
}

void OnOffGroup::turnOff()
{
    for(const shared_ptr<BasicOnOff> &d : devices){
        d->turnOff();
    }
    state = BasicOnOff::STATE::OFF;
}
