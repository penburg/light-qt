#ifndef SETTABLE_H
#define SETTABLE_H

#include <QObject>
#include <QVariant>

class Settable : public QObject
{
    Q_OBJECT
public:
    virtual bool setOption(QString name, QVariant value) = 0;

signals:
};

#endif // SETTABLE_H
