#include "nwsthermalsensor.h"

NWSThermalSensor::NWSThermalSensor(QString n, QString s, int i, double t) : ThermalSensor(n, i, t)
{
    station = s;
    manager = unique_ptr<QNetworkAccessManager>(new QNetworkAccessManager(this));
    connect(manager.get(), &QNetworkAccessManager::finished, this, &NWSThermalSensor::replyFinished);
}

void NWSThermalSensor::replyFinished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError error = reply->error();
    QString data = reply->readAll();
    if(error == QNetworkReply::NoError){
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data.toLocal8Bit());
        if(isValid(jsonDoc.object().value(Json_Temp_key))){
            QJsonObject tempKey = jsonDoc.object().value(Json_Temp_key).toObject();
            lastRead = tempKey.value(Json_value).toDouble(std::numeric_limits<double>::max());
            readComplete();
        }
        //qDebug() << jsonDoc.toJson();
    }
    else{

    }

    reply->deleteLater();
}

bool NWSThermalSensor::isValid(QJsonValue obj)
{
    if(obj != QJsonValue::Undefined){
        if(obj.isObject()){
            QJsonObject arr = obj.toObject();
            if(arr.contains(Json_qualityCode) && arr.contains(Json_value) && arr.contains(Json_unitCode)){
                QString units = arr.value(Json_unitCode).toString();
                QString quality = arr.value(Json_qualityCode).toString();
                double value = arr.value(Json_value).toDouble(std::numeric_limits<double>::max());
                if(!units.isNull() && !quality.isNull() && value != std::numeric_limits<double>::max()){
                    if(units.compare(Json_unitCode_String) == 0){
                        if(quality.compare(Json_qualityGood) == 0){
                            return true;
                        }
                        else{
                            qWarning() << "Station" << station << "invaid quality code" << quality;
                        }
                    }
                    else{
                        qWarning() << "Station" << station << "invalid units" << units;
                    }
                }
            }
        }
    }
    return false;
}

void NWSThermalSensor::readSensor()
{
    QNetworkRequest request;
    request.setUrl(QUrl(this->baseURL + station + endpoint));
    request.setRawHeader(User_Agent.toLocal8Bit(), User_Agent_String.toLocal8Bit());
    request.setRawHeader(Accept.toLocal8Bit(), Accept_String.toLocal8Bit());
    manager->get(request);
}
