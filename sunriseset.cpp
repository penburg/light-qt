#include "sunriseset.h"

SunRiseSet::SunRiseSet(double lat, double lon, int time, QObject *parent) : QObject(parent)
{
    latitude = lat;
    longitude = lon;
    timeZone = time;

    updateTime = "01:30";
    nightlyUpdateTime = "12:00";

    conditionalSunRiseTime = "";
    conditionalSunSetTime = "";

    this->sunRiseTimer = unique_ptr<QTimer>(new QTimer(this));
    connect(sunRiseTimer.get(), &QTimer::timeout, this, &SunRiseSet::privateRise);
    sunRiseTimer->setSingleShot(true);

    this->sunSetTimer = unique_ptr<QTimer>(new QTimer(this));
    connect(sunSetTimer.get(), &QTimer::timeout, this, &SunRiseSet::privateSet);
    sunSetTimer->setSingleShot(true);

    this->reCalcTimer = unique_ptr<QTimer>(new QTimer(this));
    connect(reCalcTimer.get(), &QTimer::timeout, this, &SunRiseSet::nightlyCalc);
    reCalcTimer->setSingleShot(true);

    setAction = make_shared<ActionEvent>("Sunset");
    riseAction = make_shared<ActionEvent>("Sunrise");
    conditionalSetAction = make_shared<ActionEvent>("Conditional_Sunset");
    conditionalRiseAction = make_shared<ActionEvent>("Conditional_Sunrise");


    reCalc();
}

SunRiseSet::~SunRiseSet()
{
    if(sunSetTimer->isActive()){
        sunSetTimer->stop();
    }
    if(sunRiseTimer->isActive()){
        sunRiseTimer->stop();
    }
    if(reCalcTimer->isActive()){
        reCalcTimer->stop();
    }
    sunSetTimer.reset();
    sunRiseTimer.reset();
    reCalcTimer.reset();
}

unsigned long SunRiseSet::getSunSet() const
{
    return sunSet;
}

unsigned long SunRiseSet::getSunRise() const
{
    return sunRise;
}

void SunRiseSet::setLatLon(double lat, double lon, int time)
{
    latitude = lat;
    longitude = lon;
    timeZone = time;
    reCalc();
}

void SunRiseSet::reCalc()
{
    //qDebug() << "Starting Calculations";
    std::time_t now = std::time(nullptr);
    calcRiseSet(now);
    setTimers();
}

bool SunRiseSet::isDaylight()
{
    return constIsDay();
}

bool SunRiseSet::constIsDay() const
{
    unsigned long now = std::time(nullptr);
    if(now > sunRise && now < sunSet){
        return true;
    }
    return false;
}

QString SunRiseSet::getStatus() const
{
    QString ret;
    QDateTime dateObj = QDateTime::currentDateTime();
    ret += "Current Time: " + SunRiseSet::formatTime(dateObj.toSecsSinceEpoch()) + "\n";
    ret += "Sun Rise: " + SunRiseSet::formatTime(getSunRise()) + "\n";
    ret += conditionalSunRiseTime.isEmpty() ? "" : "Emit Rise if after: " + conditionalSunRiseTime + "\n";
    ret += "Sun Set: " + SunRiseSet::formatTime(getSunSet()) + "\n";
    ret += conditionalSunSetTime.isEmpty() ? "" : "Emit Set if before: " + conditionalSunSetTime + "\n";
    ret += "It's: ";
    ret += constIsDay() ? "Day" : "Night";
    ret += "\n";
    return ret;
}

void SunRiseSet::privateRise()
{
    emit onSunRise();
    riseAction->emitEvent();
    if(validTime(conditionalSunRiseTime)){
        QDateTime contTime = QDateTime::currentDateTime();
        QDateTime now = QDateTime::currentDateTime();
        QTime time = QTime::fromString(conditionalSunRiseTime, timeFormat);
        contTime.setTime(time);

        if(now.toSecsSinceEpoch() - contTime.toSecsSinceEpoch() > 0){
            conditionalRiseAction->emitEvent();
        }
    }
}

void SunRiseSet::privateSet()
{
    emit onSunSet();
    setAction->emitEvent();
    if(validTime(conditionalSunSetTime)){
        QDateTime contTime = QDateTime::currentDateTime();
        QDateTime now = QDateTime::currentDateTime();
        QTime time = QTime::fromString(conditionalSunSetTime, timeFormat);
        contTime.setTime(time);

        if(contTime.toSecsSinceEpoch() - now.toSecsSinceEpoch() > 0){
            conditionalSetAction->emitEvent();
        }
    }
}

void SunRiseSet::setTimers()
{
    std::time_t now = std::time(nullptr);
    long timeToSunRise = sunRise - now;
    long timeToSunSet = sunSet - now;

    //qDebug() << "SunRise in:" << QString::number(timeToSunRise) << "at" << formatTime(sunRise);
    //qDebug() << "Sunset in:" << QString::number(timeToSunSet) << "at" << formatTime(sunSet);

    if(timeToSunRise > 0){
        sunRiseTimer->start(timeToSunRise * 1000);
    }
    if(timeToSunSet > 0){
        sunSetTimer->start(timeToSunSet * 1000);
    }

    QDateTime dateObj = QDateTime::currentDateTime();
    QTime reCalcTime = QTime::fromString(updateTime, "hh:mm");
    dateObj = dateObj.addDays(1);
    dateObj.setTime(reCalcTime);

    long timeToReCalc = dateObj.toSecsSinceEpoch() - now;
    //qDebug() << "Recalc in:" << QString::number(timeToReCalc);
    if(timeToReCalc > 0){
        reCalcTimer->start(timeToReCalc * 1000);
    }

    //qDebug() << "SunRise Timer Remaining:" << QString::number(sunRiseTimer->remainingTime() / 1000) << "isActive" << sunRiseTimer->isActive();
    //qDebug() << "SunSet Timer Remaining:" << QString::number(sunSetTimer->remainingTime() / 1000) << "isActive" << sunSetTimer->isActive();
    //qDebug() << "reCalc Timer Remaining:" << QString::number(reCalcTimer->remainingTime() / 1000) << "isActive" << reCalcTimer->isActive();
}

void SunRiseSet::nightlyCalc()
{
    //qDebug() << "Nightly Calculations";
    QDateTime dateObj = QDateTime::currentDateTime();
    QTime time = QTime::fromString(nightlyUpdateTime, timeFormat);
    dateObj.setTime(time);
    calcRiseSet(dateObj.toSecsSinceEpoch());
    setTimers();
}

const shared_ptr<ActionEvent> &SunRiseSet::getConditionalRiseAction() const
{
    return conditionalRiseAction;
}

bool SunRiseSet::validTime(QString time)
{
    if(!time.isEmpty() && time.size() == 5 && time.contains(':')){
        bool isHourNum, isMinNum;
        QStringList hourMin = time.split(':');
        int hour = hourMin.at(0).toInt(&isHourNum);
        int min = hourMin.at(1).toInt(&isMinNum);
        if(isHourNum && isMinNum && hour >= 0 && hour <= 24 && min >= 0 && min <= 60){
            return true;
        }
    }
    return false;
}

QJsonDocument SunRiseSet::jsonStatus() const
{
    QJsonArray ret;
    QVariantMap map;

    QDateTime dateObj = QDateTime::currentDateTime();

    map.insert(sKeyName, "Current Time");
    map.insert(sKeyValue, SunRiseSet::formatTime(dateObj.toSecsSinceEpoch()));
    map.insert(sKeyDesc, "The current time");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Sun Rise");
    map.insert(sKeyValue, SunRiseSet::formatTime(getSunRise()));
    map.insert(sKeyDesc, "Todays sun rise time");
    ret.append(QJsonObject::fromVariantMap(map));

    map.clear();
    map.insert(sKeyName, "Sun Set");
    map.insert(sKeyValue, SunRiseSet::formatTime(getSunSet()));
    map.insert(sKeyDesc, "Todays sun set time");
    ret.append(QJsonObject::fromVariantMap(map));

    if(!conditionalSunRiseTime.isEmpty()){
        map.clear();
        map.insert(sKeyName, "Conditional Sun Rise");
        map.insert(sKeyValue, conditionalSunRiseTime);
        map.insert(sKeyDesc, "Emit Rise if after this time " );
        ret.append(QJsonObject::fromVariantMap(map));
    }
    if(!conditionalSunRiseTime.isEmpty()){
        map.clear();
        map.insert(sKeyName, "Conditional Sun Set");
        map.insert(sKeyValue, conditionalSunSetTime);
        map.insert(sKeyDesc, "Emit Set if after this time ");
        ret.append(QJsonObject::fromVariantMap(map));
    }

    map.clear();
    map.insert(sKeyName, "SunStatus");
    map.insert(sKeyValue, constIsDay() ? "Day" : "Night");
    map.insert(sKeyDesc, "Is it currently Day or Night");
    ret.append(QJsonObject::fromVariantMap(map));


    return QJsonDocument(ret);
}

const shared_ptr<ActionEvent> &SunRiseSet::getConditionalSetAction() const
{
    return conditionalSetAction;
}

void SunRiseSet::setConditionalSunRiseTime(const QString &newConditionalSunRiseTime)
{
    conditionalSunRiseTime = newConditionalSunRiseTime;
    conditionalRiseAction->setExtraText("\tIf after: " + newConditionalSunRiseTime);
}

void SunRiseSet::setConditionalSunSetTime(const QString &newConditionalSunSetTime)
{
    conditionalSunSetTime = newConditionalSunSetTime;
    conditionalSetAction->setExtraText("\tIf before: " + newConditionalSunSetTime);
}

const shared_ptr<ActionEvent> &SunRiseSet::getRiseAction() const
{
    return riseAction;
}

const shared_ptr<ActionEvent> &SunRiseSet::getSetAction() const
{
    return setAction;
}

QString SunRiseSet::formatTime(unsigned long time)
{
    QString format = "yyyy-MM-dd hh:mm:ss";
    QDateTime dateObj;
    dateObj.setSecsSinceEpoch(time);
    return dateObj.toString(format);
}

void SunRiseSet::calcRiseSet(unsigned long unixTime)
{
    // Convert Julian day to Unix Timestamp
    unsigned long Jdate = (unsigned long) unixTime / 86400.0 + 2440587.5;
    // Number of days since Jan 1st, 2000 12:00
    float n = (float)Jdate - 2451545.0 + 0.0008;
    // Mean solar noon
    float Jstar = -longitude / 360 + n;
    // Solar mean anomaly
    float M = fmod((357.5291 + 0.98560028 * Jstar), 360);
    // Equation of the center
    float C = 1.9148 * sin(M / 360 * 2 * M_PI) + 0.02 * sin(2 * M / 360 * 2 * M_PI) + 0.0003 * sin(3 * M * 360 * 2 * M_PI);
    // Ecliptic longitude
    float lambda = fmod((M + C + 180 + 102.9372), 360);
    // Solar transit
    float Jtransit = Jstar + (0.0053 * sin(M / 360.0 * 2.0 * M_PI) - 0.0069 * sin(2.0 * (lambda / 360.0 * 2.0 * M_PI)));
    // Declination of the Sun
    float delta = asin(sin(lambda / 360 * 2 * M_PI) * sin(23.44 / 360 * 2 * M_PI)) / (2 * M_PI) * 360;
    // Hour angle
    float omega0 = 360 / (2 * M_PI) * acos((sin(-0.83 / 360 * 2 * M_PI) - sin(latitude / 360 * 2 * M_PI) * sin(delta / 360 * 2 * M_PI)) / (cos(latitude / 360 * 2 * M_PI) * cos(delta / 360 * 2 * M_PI)));
    // Julian day sunrise, sunset
    float Jset = Jtransit + omega0 / 360;
    float Jrise = Jtransit - omega0 / 360;
    // Convert to Unix Timestamp
    unsigned long unixRise = Jrise * 86400 + 946728000;
    unsigned long unixSet = Jset * 86400 + 946728000;
    sunRise = unixRise;
    sunSet = unixSet;
}
