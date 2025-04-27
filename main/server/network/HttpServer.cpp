#include "HttpServer.h"
#include <QTcpServer>

/*!
* Qt ֧�� GET��POST �������� HTTP ������body() �����Ե��ã��Ҳ����׳��쳣
*/


// 1. ���������ɺ��� 1)ȫ�־�̬,2)�޲��� ������
QJsonObject GlobalHandler(const QHttpServerRequest& request) {
    qDebug() << "body:" << request.body();
    return QJsonObject{ {"type", "global function"} };
}

HttpServer::~HttpServer() {
   
}

bool HttpServer::StartServer(int port) {

    qDebug() << "HTTP server starting on port" << port;
    //����GET����
    server.route("/hello", [this]() {
        // Handle GET request to /api
        qDebug() << "Received GET request to /api";
        QJsonObject response;
        response["message"] = "Hello from the HTTP server!";
        return response;
        }
    );
    server.route("/search", QHttpServerRequest::Method::Get, [](const QHttpServerRequest& request) {
        QUrlQuery query(request.url().query());
        QString keyword = query.queryItemValue("keyword");
        qDebug() << "Keyword:" << keyword;
        return QJsonObject{ {"keyword", keyword} };
        });
    // ��������GET����
    server.route("/user/<arg>", [](const QString& name) {
        QJsonObject response;
        response["message"] = QString("Hello, %1!").arg(name);
        return response;
        });

    // POST����·��
    server.route("/login", QHttpServerRequest::Method::Post,
        [](const QHttpServerRequest& request) {
            // ����JSON������
            QJsonDocument doc = QJsonDocument::fromJson(request.body());
            QJsonObject requestData = doc.object();

            // �����¼�߼�
            QString username = requestData["username"].toString();
            QString password = requestData["password"].toString();

            QJsonObject response;
            if (username == "admin" && password == "123456") {
                response["status"] = "success";
                response["message"] = "Login successful";
            } else {
                response["status"] = "error";
                response["message"] = "Invalid credentials";
            }

            return response;
        }
    );

    
    /*
    QJsonObject(*)(const QHttpServerRequest&)���˴�Ϊ����ָ�� void (*funcPtr)() = myFunction;  // �Ϸ�����ʽת��
    void (*funcPtr)() = &myFunction; // �Ϸ�����ʽȡ��ַ
    */
    // 1. ��ȫ�ֺ��� 
    server.route("/global", QHttpServerRequest::Method::Get,GlobalHandler);//����&Ҳ����ʹ��
    // 2. �󶨾�̬��Ա����
    server.route("/static", QHttpServerRequest::Method::Get,&HttpServer::staticHandler);

    // 3. ֱ�Ӱ󶨵���Ա���� Lambda��ʽ
    server.route("/get", QHttpServerRequest::Method::Get,
        [this](const QHttpServerRequest &request) {
            return handle_request(request);
        });

    auto tcpserver = std::make_unique<QTcpServer>();
    if (!tcpserver->listen(QHostAddress::Any, port)) {
        qDebug() << "Failed to start server:" << tcpserver->errorString();
        return false;
    }
    if (!server.bind(tcpserver.get())) {
        qDebug() << "Failed to start HTTP server";
        return false;
    }
    quint16 port_tcp = tcpserver->serverPort();
    tcpserver.release(); // Transfer ownership to QHttpServer �ͷ� std::unique_ptr ������Ȩ,ת������Ȩ
    qDebug() << tr("HTTP server Running on http://127.0.0.1:%1/").arg(port_tcp);
    return true;
}

QJsonObject HttpServer::handle_request(const QHttpServerRequest& request) {
    qDebug() << "body:"<<request.body();
    QJsonObject response;
    response["data"] = "Some data from member function";
    return response;
}
