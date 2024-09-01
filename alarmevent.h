#ifndef ALARMEVENT_H
#define ALARMEVENT_H

#include "actionevent.h"
#include <QTimer>
#include <memory>
#include <QDateTime>

using namespace std;

class AlarmEvent : public ActionEvent
{
    Q_OBJECT
public:
    AlarmEvent(QString name, QString t);
    void setDayNight(bool isDay);

    // Statusable interface
    QJsonDocument jsonStatus() const override;
    QString getStatus() const override;

signals:
    bool isDaylight();

private:
    void setAlarm();
    void privateOnTimeout();
    unique_ptr<QTimer> timer;
    QString time;
    QString timeFormat = "hh:mm";
    bool conditionalDayNight;
    bool dayNightCondition;


};

#endif // ALARMEVENT_H
