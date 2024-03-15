#include "sysfsthermalsensor.h"

SysfsThermalSensor::SysfsThermalSensor(QString n, QString p, int i, double t) : ThermalSensor(n, i, t)
{
    path = p;
}

void SysfsThermalSensor::readSensor()
{
    QFile sensor(path);
    //qDebug() << "Reading:" << path;
    if(sensor.open(QIODevice::ReadOnly)){
        QString value = sensor.readAll();
        bool isNumeric;
        double doubleValue = value.toDouble(&isNumeric);
        if(isNumeric && doubleValue / 1000.0 != errorValue){
            lastRead = doubleValue / 1000.0;
            errorCount = 0;
            readComplete();
        }
        else if (errorCount > 4){
            qWarning() << "Read value error" << value << "on" << name;
        }
        else{
            errorCount++;
            readSensor();
        }
        sensor.close();
    }
    else{
        qCritical("Unable to read Thermal Sensor \"%s\"", path.toStdString().c_str() );
    }
}
