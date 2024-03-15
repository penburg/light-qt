#include "gpioinputthread.h"


GpioInputThread::GpioInputThread(QString n, gpiod::line l, EDGE e,  bool activeLow, QObject *parent)
    : QThread{parent}
{
    line = unique_ptr<gpiod::line>(new gpiod::line(l));
    name = n;
    if(!line->is_used()){
        int flags = activeLow ? gpiod::line_request::FLAG_ACTIVE_LOW.to_ulong() : 0;

        line->request({n.toStdString(), convertEdge(e), flags } );
        enabled = true;
        start();
    }

}

GpioInputThread::~GpioInputThread()
{
    //line->release();
}

void GpioInputThread::run()
{
    qDebug() << "Starting" + name + "Input Thread";
    while(enabled && !this->isInterruptionRequested()){
        auto event = line->event_wait(::std::chrono::seconds(1));
        if (event) {
            gpiod::line_event lineEvent = line->event_read();
            if (lineEvent.event_type == ::gpiod::line_event::RISING_EDGE){
                emit onEvent(name + " - Rising Edge");
                if(state != 1){
                    state = 1;
                    triggerState();
                }
            }
            else if (lineEvent.event_type == ::gpiod::line_event::FALLING_EDGE){
                emit onEvent(name + " - Falling Edge");
                if(state != 0){
                    state = 0;
                    triggerState();
                }
            }
            else{
                emit onEvent(name + " - Unknown Edge");
            }
        }
    }
}

void GpioInputThread::shutdown()
{
    qDebug() << "Stopping" + name + "Input Thread";
    enabled = false;
    line->reset();
    exit(EXIT_SUCCESS);
}

QString GpioInputThread::getStatus() const
{
    QString ret = "";
    ret += "Line Value: ";
    ret += getValue() == 1 ? "Active" : "Inactive";
    ret += "\n";
    return ret;
}

int GpioInputThread::getValue() const
{
    return line->get_value();
}

void GpioInputThread::triggerState()
{
    if(state == 1){
        emit onActive();
    }
    else if(state == 0){
        emit onInactive();
    }
}

int GpioInputThread::convertEdge(EDGE e)
{
    switch(e){
    case RISING:
        return gpiod::line_request::EVENT_RISING_EDGE;
        break;
    case FALLING:
        return gpiod::line_request::EVENT_FALLING_EDGE;
        break;
    case BOTH:
        return gpiod::line_request::EVENT_BOTH_EDGES;
        break;
    }
    return -1;
}
