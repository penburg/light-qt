#include "apihandler.h"

ApiHandler::ApiHandler(QObject *parent) : QObject(parent)
{
    listener = http_listener("http://0.0.0.0:1862/");
    listener.support(methods::GET,  bind(&ApiHandler::handle_get, this, std::placeholders::_1));
    listener.support(methods::POST, bind(&ApiHandler::handle_post, this, std::placeholders::_1));

    try
    {
        listener
                .open()
                .then([]() {qDebug() << "Starting API Handler"; })
                .wait();


    }
    catch (exception const & e)
    {
        cout << e.what() << endl;
    }
}

ApiHandler::~ApiHandler()
{
    //listener.close();
    qDebug() << "Destroyed";
}

void ApiHandler::shutdown()
{
    qDebug() << "Stopping API Handler";
    listener.close();
}

void ApiHandler::display_json(const json::value &jvalue, const utility::string_t &prefix)
{
    cout << prefix << jvalue.serialize() << endl;
}

void ApiHandler::handle_get(http_request request)
{
    if(isAuthorized(request.headers())){

        auto answer = json::value::object();
        answer["status"] = json::value::string("ok");
        answer["auth"] = json::value::boolean(isAuthorized(request.headers()));
        request.reply(status_codes::OK, answer);

    }

    else{
        request.reply(status_codes::Forbidden);
    }
}

void ApiHandler::handle_post(http_request request)
{
    qInfo() << "Post Request";

    auto answer = json::value::object();
    answer["status"] = json::value::string("ok");
    request.reply(status_codes::OK, answer);
}

bool ApiHandler::isAuthorized(http_headers header)
{
    if(header.has(USER_AGENT) && header[USER_AGENT].compare(USER_AGENT_STRING) == 0){
        if(header.has(NONCE)){
            string nonce = NONCE + ": " + header[NONCE];
            string s = nonce + "FFMPEG Convert client v 1" + nonce + "Davros rules all";
            QCryptographicHash md5(QCryptographicHash::Md5);
            QByteArrayView data(s.c_str(), s.length());
            //md5.addData(s.c_str(), s.length());
            md5.addData(data);
            QString auth(md5.result().toHex());
            if(header.has(AUTH) && header[AUTH].compare(auth.toStdString()) == 0){
                //qDebug() << "Authorized Request";
                return true;
            }

        }// end nonce
    }// end user_agent comp
    qDebug() << "Access Denied";
    return false;
}
