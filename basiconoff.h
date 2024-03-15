#ifndef BASICONOFF_H
#define BASICONOFF_H

#include <QObject>
#include <QDebug>
#include <QMetaEnum>

#include "statusable.h"

class BasicOnOff : public QObject, public Statusable
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
    virtual QString getStatus() const override;
    STATE getState() const;
    const QString &getName() const;

public slots:
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    virtual void setAuto();


protected:
    STATE state;
    QString name;
};

#endif // BASICONOFF_H
