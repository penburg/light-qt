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
