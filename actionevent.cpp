#include "actionevent.h"

ActionEvent::ActionEvent(QString n)
{
    name = n;
    extraText = "";
}

void ActionEvent::emitEvent()
{
    emit onEvent(name);
}

QString ActionEvent::getStatus() const
{
    QString ret = "Name: " + name + "\n";
    if(!extraText.isEmpty()){
        ret += extraText + "\n";
    }
    return ret;
}

const QString &ActionEvent::getName() const
{
    return name;
}

void ActionEvent::setExtraText(const QString &newExtraText)
{
    extraText = newExtraText;
}

QJsonDocument ActionEvent::jsonStatus() const
{
    QJsonArray ret;
    QVariantMap map;

    map.insert(sKeyName, "Name");
    map.insert(sKeyValue, name);
    map.insert(sKeyDesc, "The event name");
    ret.append(QJsonObject::fromVariantMap(map));

    if(!extraText.isEmpty()){
        map.clear();
        map.insert(sKeyName, "ExtraText");
        map.insert(sKeyValue, extraText);
        map.insert(sKeyDesc, "The extra text for the event");
        ret.append(QJsonObject::fromVariantMap(map));
    }


    return QJsonDocument(ret);
}
