#ifndef NWSTHERMALSENSOR_H
#define NWSTHERMALSENSOR_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

#include <memory>

#include "thermalsensor.h"

class NWSThermalSensor : public ThermalSensor
{
public:
    NWSThermalSensor(QString n, QString s, int i, double t);

public slots:
    void readSensor();

private:
    QString station;
    unique_ptr<QNetworkAccessManager> manager;

    QString baseURL = "https://api.weather.gov/stations/";
    QString endpoint = "/observations/latest";
    QString User_Agent = "User-Agent";
    QString User_Agent_String = "Tempestas (Goddess of Storms)";
    QString Accept = "Accept";
    QString Accept_String = "application/ld+json";

    QString Json_Temp_key = "temperature";
    QString Json_unitCode = "unitCode";
    QString Json_unitCode_String = "wmoUnit:degC";
    QString Json_qualityCode = "qualityControl";
    QString Json_qualityGood = "V";
    QString Json_qualityBad = "Z";
    QString Json_value = "value";

    void replyFinished(QNetworkReply *reply);
    bool isValid(QJsonValue obj);
};

#endif // NWSTHERMALSENSOR_H
