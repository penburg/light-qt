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

QJsonDocument OnOffGroup::jsonStatus() const
{
    QJsonArray ret = BasicOnOff::jsonStatus().array();
    QVariantMap map;
    for(const shared_ptr<BasicOnOff> &d : devices){
        map.clear();
        map.insert(sKeyName, d->getName());
        map.insert(sKeyValue, d->jsonStatus());
        map.insert(sKeyDesc, "Controlled device status");
        ret.append(QJsonObject::fromVariantMap(map));
    }

    return QJsonDocument(ret);
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
