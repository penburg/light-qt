#ifndef STATUSABLE_H
#define STATUSABLE_H

#include <QObject>

class Statusable
{

public:
    virtual QString getStatus() const;

signals:

};

#endif // STATUSABLE_H
