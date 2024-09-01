#ifndef THERMALALERT_H
#define THERMALALERT_H

#include <QObject>
#include <QMetaEnum>
#include <QJsonArray>
#include <QJsonObject>
#include <memory>
#include "actionevent.h"
#include "alerttype.h"
#include "thermalsensor.h"
#include "statusable.h"

using namespace std;

class ThermalAlert : public QObject, public Statusable
{
    Q_OBJECT
public:
    explicit ThermalAlert(QString n, double threshold, bool greater, QObject *parent = nullptr);
    void setWarningTemp(double newWarningTemp);
    void setCriticalTemp(double newCriticalTemp);
    const shared_ptr<ActionEvent> &getAlarmedEvent() const;
    const shared_ptr<ActionEvent> &getWarningEvent() const;
    const shared_ptr<ActionEvent> &getCriticalEvent() const;
    const shared_ptr<ActionEvent> &getNormalizedEvent() const;

    // Statusable interface
    QString getStatus() const override;
    QJsonDocument jsonStatus() const override;

public slots:
    void tempChanged(double temp);



signals:
    void onAlert(QString msg, AlertType::Thermal type);

private:
    QString name;
    double lastTemp;
    double thresholdTemp;
    double warningTemp;
    double criticalTemp;
    bool greaterThan;
    shared_ptr<ActionEvent> alarmedEvent;
    shared_ptr<ActionEvent> warningEvent = nullptr;
    shared_ptr<ActionEvent> criticalEvent = nullptr;
    shared_ptr<ActionEvent> normalizedEvent;

    AlertType::Thermal activeAlert;
    AlertType::Thermal checkTemp(double left, double right, AlertType::Thermal ret);
    void sendActionEvent(AlertType::Thermal type);





};

#endif // THERMALALERT_H
