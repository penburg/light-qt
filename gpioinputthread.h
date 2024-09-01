#ifndef GPIOINPUTTHREAD_H
#define GPIOINPUTTHREAD_H

#include <QThread>
#include <QObject>
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <gpiod.hpp>
#include "statusable.h"
#include <memory>

using namespace std;

class GpioInputThread : public QThread, public Statusable
{
    Q_OBJECT
public:
    enum EDGE{
        RISING,
        FALLING,
        BOTH
    };

    explicit GpioInputThread(QString n, gpiod::line l, EDGE e, bool activeLow = false, QObject *parent = nullptr);
    ~GpioInputThread();
    void run() override;
    void shutdown();
    int getValue() const;
    void triggerState();

    // Statusable interface
    QJsonDocument jsonStatus() const override;
    QString getStatus() const override;

signals:
    void onEvent(QString name);
    void onActive();
    void onInactive();



private:
    unique_ptr<gpiod::line> line;
    QString name;
    bool enabled;
    int convertEdge(EDGE e);
    EDGE edge;
    int state;
};

#endif // GPIOINPUTTHREAD_H
