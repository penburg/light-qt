#ifndef BASICONOFF_H
#define BASICONOFF_H

#include <QObject>
#include <QDebug>
#include <QMetaEnum>
#include <QJsonArray>
#include <QJsonObject>

#include "statusable.h"
#include "settable.h"

class BasicOnOff : public QObject, public Statusable, public Settable
{
    Q_OBJECT
public:
    enum STATE{
        ON,
        OFF,
        AUTO
    };

    Q_ENUM(STATE)
    BasicOnOff(QString n);

    STATE getState() const;
    const QString &getName() const;

    // Settable interface
    bool setOption(QString name, QVariant value) override;
    QJsonDocument lsOptions() override;

    // Statusable interface
    QJsonDocument jsonStatus() const override;
    virtual QString getStatus() const override;

public slots:
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    virtual void setAuto();


protected:
    STATE state;
    QString name;




};

#endif // BASICONOFF_H
