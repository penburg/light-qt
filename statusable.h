#ifndef STATUSABLE_H
#define STATUSABLE_H

#include <QObject>
#include <QJsonDocument>

class Statusable
{

public:
    virtual QString getStatus() const;
    virtual QJsonDocument jsonStatus() const = 0;

signals:

protected:
    inline static const QString sKeyName = "Name";
    inline static const QString sKeyValue = "Value";
    inline static const QString sKeyDesc = "Desc";
};

#endif // STATUSABLE_H
