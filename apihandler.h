#ifndef APIHANDLER_H
#define APIHANDLER_H

#include <QObject>
#include <QDebug>
#include <QSettings>
#include <QCryptographicHash>

#include <cpprest/http_listener.h>
#include <cpprest/json.h>


using namespace std;
using namespace web;
using namespace web::http::experimental::listener;
using namespace web::http;

class ApiHandler : public QObject
{
    Q_OBJECT
public:
    explicit ApiHandler(QObject *parent = nullptr);

    ~ApiHandler();

public slots:
    void shutdown();

signals:

private:
    void display_json(json::value const & jvalue, utility::string_t const & prefix);
    void handle_get(http_request request);
    void handle_post(http_request request);
    bool isAuthorized(http_headers header);

    http_listener listener;
    QSettings settings;

    string USER_AGENT = "User-Agent";
    string NONCE = "X-Nonce";
    string AUTH = "X-Auth";
    string USER_AGENT_STRING = "Katniss Everdeen (Mockingjay of the Rebellion, Tribute)";
};

#endif // APIHANDLER_H
