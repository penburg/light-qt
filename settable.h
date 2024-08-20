#ifndef SETTABLE_H
#define SETTABLE_H

#include <QString>
#include <QVariant>

class Settable
{
public:
    virtual bool setOption(QString name, QVariant value) = 0;

signals:
};

#endif // SETTABLE_H
