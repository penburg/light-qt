#ifndef ALERTTYPE_H
#define ALERTTYPE_H

#include <QObject>
#include <QMetaEnum>

class AlertType : public QObject{
    Q_OBJECT
public:
    enum Thermal{
        None,
        Normalized,
        Low,
        High,
        Warning,
        Critical
    };
    Q_ENUM(Thermal)
};

#endif // ALERTTYPE_H
