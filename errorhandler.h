#ifndef ERRORHANDLER_H
#define ERRORHANDLER_H
#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>

void errorHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    //const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "\033[32mDebug\033[0m: \033[34m%s\033[0m - %s\n", function, localMsg.constData());
        break;
    case QtInfoMsg:
        fprintf(stderr, "\033[37;1mInfo\033[0m: \033[34m%s\033[0m - %s\n", function, localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "\033[1;33mWarning\033[0m: \033[34m%s\033[0m - %s\n", function, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "\033[31mCritical\033[0m: \033[34m%s\033[0m - %s\n", function, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "\033[31mFatal\033[0m: \033[34m%s\033[0m - %s\n", function, localMsg.constData());
        break;
    }
}
#endif // ERRORHANDLER_H
