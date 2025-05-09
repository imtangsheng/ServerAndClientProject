#pragma once
#include <QHttpServer>

class HttpServer : public QObject
{
public:
    HttpServer() = default;
    ~HttpServer();

    bool StartServer(int port=80);

    QHttpServer server;
    QJsonObject handle_request(const QHttpServerRequest& request);
};
