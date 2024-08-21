#ifndef SETTABLE_H
#define SETTABLE_H

#include <QString>
#include <QVariant>
#include <QJsonDocument>

class Settable
{
public:
    virtual bool setOption(QString name, QVariant value) = 0;
    virtual QJsonDocument lsOptions() = 0;

    inline static const QString keyName = "Name";
    inline static const QString keyValueType = "Type";
    inline static const QString keyDesc = "Desc";

};

#endif // SETTABLE_H
