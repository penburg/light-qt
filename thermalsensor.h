#ifndef THERMALSENSOR_H
#define THERMALSENSOR_H

#include <QObject>
#include <QTimer>
#include <QFile>
#include <QSettings>
#include <QDebug>
#include <memory>

#include "statusable.h"

using namespace std;

class ThermalSensor : public QObject, public Statusable
{
    Q_OBJECT


public:
    explicit ThermalSensor(QString n, int i = 300000, double t = 1.0);
    ~ThermalSensor();
    virtual void shutdown();
    double getLastRead() const;
    static QString getFormatedTemp(double t);
    static QString getFormatedValue(double t);
    virtual QString getStatus() const;

    void setErrorValue(int newErrorValue);

public slots:
    virtual void readSensor();
    void reportNow();

signals:
    void tempChanged(double temp);

protected:
    QString name;
    double lastRead;
    void readComplete();
    int errorCount;
    double errorValue;

private:
    int interval;
    double threashold;
    double lastReported;



    unique_ptr<QTimer> timer;
};

#endif // THERMALSENSOR_H
