#ifndef EVENTTOACTION_H
#define EVENTTOACTION_H

#include <QObject>
#include <QMetaEnum>
#include <memory>
#include "statusable.h"
#include "actionevent.h"
#include "basiconoff.h"

using namespace std;

class EventToAction : public QObject, public Statusable
{
    Q_OBJECT
public:
    explicit EventToAction(QString n, shared_ptr<ActionEvent> e, BasicOnOff::STATE s, shared_ptr<BasicOnOff> a, shared_ptr<QHash<QString, shared_ptr<EventToAction>>> p, QObject *pnt = nullptr);
    void setOneShot(bool shot = true);

public slots:
    QString getStatus() const;

signals:
    void onComplete(QString msg);

private:
    shared_ptr<ActionEvent> event;
    shared_ptr<BasicOnOff> action;
    BasicOnOff::STATE state;
    shared_ptr<QHash<QString, shared_ptr<EventToAction>>> parent;
    bool oneShot;
    QString name;

private slots:
    void onAction(QString eventName);

};

#endif // EVENTTOACTION_H
