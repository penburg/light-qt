#ifndef ONOFFGROUP_H
#define ONOFFGROUP_H

#include <memory>
#include <vector>
#include "basiconoff.h"

using namespace std;

class OnOffGroup : public BasicOnOff
{
public:
    OnOffGroup(QString name);
    QString getStatus() const;
    void addDevice(shared_ptr<BasicOnOff> d);

public slots:
    void turnOn();
    void turnOff();

private:
    vector<shared_ptr<BasicOnOff>> devices;


};

#endif // ONOFFGROUP_H
