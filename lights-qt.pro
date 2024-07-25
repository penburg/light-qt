QT -= gui
QT += network

CONFIG += c++17 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        actionevent.cpp \
        alarmevent.cpp \
        alertlogger.cpp \
        apihandler.cpp \
        basiconoff.cpp \
        console.cpp \
        evapcooler.cpp \
        eventtoaction.cpp \
        gpioinput.cpp \
        gpioinputthread.cpp \
        gpiooutput.cpp \
        gpiopwm.cpp \
        lights.cpp \
        main.cpp \
        nwsthermalsensor.cpp \
        onoffgroup.cpp \
        statusable.cpp \
        sunriseset.cpp \
        sysfsthermalsensor.cpp \
        thermalalert.cpp \
        thermalsensor.cpp \
        thermostat.cpp \
        virtualio.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    actionevent.h \
    alarmevent.h \
    alertlogger.h \
    alerttype.h \
    apihandler.h \
    basiconoff.h \
    console.h \
    errorhandler.h \
    evapcooler.h \
    eventtoaction.h \
    gpioinput.h \
    gpioinputthread.h \
    gpiooutput.h \
    gpiopwm.h \
    lights.h \
    nwsthermalsensor.h \
    onoffgroup.h \
    statusable.h \
    sunriseset.h \
    sysfsthermalsensor.h \
    thermalalert.h \
    thermalsensor.h \
    thermostat.h \
    virtualio.h

unix: INCLUDEPATH += "{1}"
unix: LIBS += -lcrypto
unix: LIBS += -lcpprest
unix: LIBS += -lgpiodcxx
