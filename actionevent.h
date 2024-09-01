#ifndef ACTIONEVENT_H
#define ACTIONEVENT_H

#include <QObject>
#include <QJsonArray>
#include <QJsonObject>

#include "statusable.h"

class ActionEvent : public QObject, public Statusable
{
    Q_OBJECT
public:
    ActionEvent(QString n);
    void emitEvent();
    const QString &getName() const;
    void setExtraText(const QString &newExtraText);

    // Statusable interface
    QJsonDocument jsonStatus() const override;
    QString getStatus() const override;

signals:
    void onEvent(QString name);

private:
    QString name;
    QString extraText;

};

#endif // ACTIONEVENT_H
