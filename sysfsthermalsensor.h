#ifndef SYSFSTHERMALSENSOR_H
#define SYSFSTHERMALSENSOR_H

#include "thermalsensor.h"

class SysfsThermalSensor : public ThermalSensor
{
public:
    SysfsThermalSensor(QString n, QString p, int i, double t);

public slots:
    void readSensor();

private:
    QString path;
};

#endif // SYSFSTHERMALSENSOR_H
