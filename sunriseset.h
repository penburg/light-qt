#ifndef SUNSET_H
#define SUNSET_H

#include <QObject>
#include <math.h>
#include <ctime>
#include <QTimer>
#include <memory>
#include <QDateTime>
#include <QDebug>

#include "actionevent.h"

using namespace std;

class SunRiseSet : public QObject, public Statusable
{
    Q_OBJECT
public:
    explicit SunRiseSet(double lat = 40, double lon = -105, int time = -7, QObject *parent = nullptr);
    ~SunRiseSet();

    void calcRiseSet(unsigned long unixTime);
    unsigned long getSunSet() const;
    unsigned long getSunRise() const;
    void setLatLon(double lat, double lon, int time);
    static QString formatTime(unsigned long time);
    const shared_ptr<ActionEvent> &getSetAction() const;
    const shared_ptr<ActionEvent> &getRiseAction() const;
    void setConditionalSunSetTime(const QString &newConditionalSunSetTime);
    void setConditionalSunRiseTime(const QString &newConditionalSunRiseTime);
    const shared_ptr<ActionEvent> &getConditionalSetAction() const;
    const shared_ptr<ActionEvent> &getConditionalRiseAction() const;

    static bool validTime(QString time);
    QString getStatus() const;

public slots:
    void reCalc();
    bool isDaylight();


signals:
    void onSunRise();
    void onSunSet();

private:
    bool constIsDay() const;
    QString timeFormat = "hh:mm";
    void privateRise();
    void privateSet();
    void setTimers();
    void nightlyCalc();
    double latitude;
    double longitude;
    int timeZone;
    QString updateTime;
    QString nightlyUpdateTime;
    QString conditionalSunSetTime;
    QString conditionalSunRiseTime;
    unsigned long sunSet;
    unsigned long sunRise;
    unique_ptr<QTimer> sunRiseTimer;
    unique_ptr<QTimer> sunSetTimer;
    unique_ptr<QTimer> reCalcTimer;
    shared_ptr<ActionEvent> setAction;
    shared_ptr<ActionEvent> riseAction;
    shared_ptr<ActionEvent> conditionalSetAction;
    shared_ptr<ActionEvent> conditionalRiseAction;



};

#endif // SUNSET_H
