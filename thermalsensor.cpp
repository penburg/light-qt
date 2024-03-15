#include "thermalsensor.h"

ThermalSensor::ThermalSensor(QString n, int i, double t)
{
    this->name = n;
    this->interval = i;
    this->threashold = t;
    this->lastRead = std::numeric_limits<double>::max();
    this->lastReported =  std::numeric_limits<double>::max();
    this->errorValue = std::numeric_limits<double>::max();
    this->errorCount = 0;

    this->timer = unique_ptr<QTimer>(new QTimer(this));
    connect(timer.get(), &QTimer::timeout, this, &ThermalSensor::readSensor);
    timer->start(interval * 1000);

}

ThermalSensor::~ThermalSensor()
{
    if(timer->isActive()){
        timer->stop();

    }
    timer.reset();
}


void ThermalSensor::shutdown()
{
    if(timer->isActive()){
        timer->stop();
    }
}

double ThermalSensor::getLastRead() const
{
    return lastRead;
}

QString ThermalSensor::getFormatedTemp(double t)
{
    QSettings settings;
    bool isF = settings.value("useFahrenheit", false).toBool();
    double temp = t;
    QString unit = "c";
    if(isF){
        temp = (t * (9.0/5.0)) + 32.0;
        unit = "f";
    }
    return QString::number(temp) + "°" + unit;
}

QString ThermalSensor::getFormatedValue(double t)
{
    QSettings settings;
    bool isF = settings.value("useFahrenheit", false).toBool();
    double temp = t;
    QString unit = "c";
    if(isF){
        temp = (t * (9.0/5.0));
        unit = "f";
    }
    return QString::number(temp) + "°" + unit;
}

void ThermalSensor::readComplete()
{
    //qDebug() << "Last:" << lastRead << "LastRepored:" << lastReported << "abs:" << abs(lastRead - lastReported) << "Threshold:" << threashold;
    if(abs(lastRead - lastReported) > threashold){
        emit tempChanged(lastRead);
        lastReported = lastRead;
        //qDebug() << "Report Sent";
    }
    else{
        //qDebug() << "Not Reporting";
    }
}

void ThermalSensor::setErrorValue(int newErrorValue)
{
    errorValue = newErrorValue;
}

void ThermalSensor::readSensor()
{
    qWarning() << "Reading sensor from base class";
}

QString ThermalSensor::getStatus() const
{
    return name + ": " + getFormatedTemp(lastRead) + "\n";
}

void ThermalSensor::reportNow()
{
    emit tempChanged(lastRead);
}


