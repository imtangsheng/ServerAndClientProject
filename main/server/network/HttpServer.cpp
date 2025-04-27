#include "HttpServer.h"
#include <QTcpServer>

/*!
* Qt 支持 GET、POST 还是其他 HTTP 方法，body() 都可以调用，且不会抛出异常
*/


// 1. 独立的自由函数 1)全局静态,2)无参数 都可以
QJsonObject GlobalHandler(const QHttpServerRequest& request) {
    qDebug() << "body:" << request.body();
    return QJsonObject{ {"type", "global function"} };
}

HttpServer::~HttpServer() {
   
}

bool HttpServer::StartServer(int port) {

    qDebug() << "HTTP server starting on port" << port;
    //处理GET请求
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
    // 带参数的GET请求
    server.route("/user/<arg>", [](const QString& name) {
        QJsonObject response;
        response["message"] = QString("Hello, %1!").arg(name);
        return response;
        });

    // POST请求路由
    server.route("/login", QHttpServerRequest::Method::Post,
        [](const QHttpServerRequest& request) {
            // 解析JSON请求体
            QJsonDocument doc = QJsonDocument::fromJson(request.body());
            QJsonObject requestData = doc.object();

            // 处理登录逻辑
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
    QJsonObject(*)(const QHttpServerRequest&)）此处为函数指针 void (*funcPtr)() = myFunction;  // 合法，隐式转换
    void (*funcPtr)() = &myFunction; // 合法，显式取地址
    */
    // 1. 绑定全局函数 
    server.route("/global", QHttpServerRequest::Method::Get,GlobalHandler);//不加&也可以使用
    // 2. 绑定静态成员函数
    server.route("/static", QHttpServerRequest::Method::Get,&HttpServer::staticHandler);

    // 3. 直接绑定到成员函数 Lambda方式
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
    tcpserver.release(); // Transfer ownership to QHttpServer 释放 std::unique_ptr 的所有权,转移所有权
    qDebug() << tr("HTTP server Running on http://127.0.0.1:%1/").arg(port_tcp);
    return true;
}

QJsonObject HttpServer::handle_request(const QHttpServerRequest& request) {
    qDebug() << "body:"<<request.body();
    QJsonObject response;
    response["data"] = "Some data from member function";
    return response;
}
