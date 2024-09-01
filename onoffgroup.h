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
    void addDevice(shared_ptr<BasicOnOff> d);

    // Statusable interface
    QJsonDocument jsonStatus() const override;
    QString getStatus() const override;

public slots:
    void turnOn() override;
    void turnOff() override;

private:
    vector<shared_ptr<BasicOnOff>> devices;
};

#endif // ONOFFGROUP_H
