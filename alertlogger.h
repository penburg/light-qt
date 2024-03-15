#ifndef ALERTLOGGER_H
#define ALERTLOGGER_H


#include <QObject>
#include <QDateTime>
#include <iostream>
#include "alerttype.h"

using namespace std;

class AlertLogger : public QObject
{
    Q_OBJECT
public:
    AlertLogger(ostream *o = &cout, QObject *parent = nullptr);

public slots:
    void onThermalAlert(QString msg, AlertType::Thermal type);
    void onSunRise();
    void onSunSet();
    void onConditionalSunRise();
    void onConditionalSunSet();
    void onThermostatOn(QString name);
    void onThermostatOff(QString name);
    void onActionEvent(QString name);
    void onGeneric(QString msg);

signals:

private:
    std::ostream *out;
    string getTimestamp();

};

#endif // ALERTLOGGER_H
