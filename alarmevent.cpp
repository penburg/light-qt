#include "alarmevent.h"

AlarmEvent::AlarmEvent(QString name, QString t): ActionEvent(name)
{
    timer = unique_ptr<QTimer>(new QTimer);
    timer->setSingleShot(true);
    connect(timer.get(), &QTimer::timeout, this, &AlarmEvent::privateOnTimeout);
    time= t;
    setAlarm();
    conditionalDayNight = false;
}

QString AlarmEvent::getStatus() const
{
    QString ret = ActionEvent::getStatus();
    ret += "\tAlarm set for: " + time + "\n";
    ret += "\tTime to alarm: " + QString::number(timer->remainingTime() / 1000) + "\n";
    if(conditionalDayNight){
        ret += dayNightCondition ? "\tIf Daytime\n" : "\tif Nighttime\n";
    }

    return ret;
}

void AlarmEvent::setAlarm()
{
    QDateTime dateObj = QDateTime::currentDateTime();
    QDateTime now = QDateTime::currentDateTime();
    QTime alarmTime = QTime::fromString(time, timeFormat);
    dateObj.setTime(alarmTime);
    long timeTillAlarm = dateObj.toSecsSinceEpoch() - now.toSecsSinceEpoch();

    if(timeTillAlarm < 3){
        long day = 60 * 60 * 24;
        timeTillAlarm += day;
    }
    timer->start(timeTillAlarm * 1000);
}

void AlarmEvent::privateOnTimeout()
{
    setAlarm();
    if(conditionalDayNight == false){
        emitEvent();
    }
    else{
        if(emit isDaylight() == dayNightCondition){
            emitEvent();
        }
    }

}

void AlarmEvent::setDayNight(bool isDay)
{
    conditionalDayNight = true;
    dayNightCondition = isDay;
}

