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

    // 2. 静态成员函数
    static QJsonObject staticHandler(const QHttpServerRequest& request) {
        qDebug() << "staticHandler"<<request.body();
        return QJsonObject{ {"type", "static method"} };
    }

    // 3. 普通成员函数
    QJsonObject memberHandler(const QHttpServerRequest& request) {
        return QJsonObject{ {"type", "member method"} };
    }
};
