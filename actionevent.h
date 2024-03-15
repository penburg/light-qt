#ifndef ACTIONEVENT_H
#define ACTIONEVENT_H

#include <QObject>

#include "statusable.h"

class ActionEvent : public QObject, public Statusable
{
    Q_OBJECT
public:
    ActionEvent(QString n);
    void emitEvent();
    QString getStatus() const;
    const QString &getName() const;

    void setExtraText(const QString &newExtraText);

signals:
    void onEvent(QString name);

private:
    QString name;
    QString extraText;
};

#endif // ACTIONEVENT_H
