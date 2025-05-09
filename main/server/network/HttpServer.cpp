#include "HttpServer.h"
#include <QTcpServer>
#include <QCoreApplication>
#include <QProcess>
#include <QTimer>
class HttpSession
{
public:
    static qint64 GetTimestamp() {
        return QDateTime::currentDateTime().toMSecsSinceEpoch();
    }
    static QJsonObject Response(quint8 status=0, QJsonValue data="") {
        return QJsonObject{
            {"status", status},
            {"data", data},
            {"timestamp", GetTimestamp()}
        };
    }
};

/*!
* Qt 支持 GET、POST 还是其他 HTTP 方法，body() 都可以调用，且不会抛出异常
* 独立的自由函数 1)全局静态,2)无参数 都可以
* 绑定到成员函数 Lambda方式
QJsonObject(*)(const QHttpServerRequest&)）此处为函数指针 void (*funcPtr)() = myFunction;  // 合法，隐式转换
void (*funcPtr)() = &myFunction; // 合法，显式取地址
*/
static QJsonObject RestartApp() {
    //使用系统特定的重启方法（跨平台）
    // 延迟重启,否则会立马重启
    QTimer::singleShot(100, []() {
        QStringList args = QCoreApplication::arguments();
        QString program = args.takeFirst();
        QProcess::startDetached(program, args);
        QCoreApplication::exit();
        });

    return HttpSession::Response(0, "restart app");
}
QJsonObject GlobalHandler(const QHttpServerRequest& request) {
    qDebug() << "body:" << request.body();
    return QJsonObject{ {"type", "global function"} };
}

HttpServer::~HttpServer() {
   
}
#include <QHttpServerResponse>
bool HttpServer::StartServer(int port) {

    server.addAfterRequestHandler(&server, [](const QHttpServerRequest&, QHttpServerResponse& resp) {
        qDebug() << "addAfterRequestHandler";
        auto h = resp.headers();
        h.append(QHttpHeaders::WellKnownHeader::AccessControlAllowOrigin, "*");
        resp.setHeaders(std::move(h));
    });
    qDebug() << "HTTP server starting on port" << port;
    /*处理GET请求*/
    server.route("/restart", QHttpServerRequest::Method::Get, RestartApp);
    server.route("/search", QHttpServerRequest::Method::Get, [](const QHttpServerRequest& request) {
        QUrlQuery query(request.url().query());
        QString keyword = query.queryItemValue("keyword");
        qDebug() << "Keyword:" << keyword;
        return QJsonObject{ {"keyword", keyword} };
        });
    server.route("/user/<arg>", [](const QString& name) {
        qDebug() << "Received GET request to /user/" << name;
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

    server.route("/get", QHttpServerRequest::Method::Get,
        [this](const QHttpServerRequest &request) {
            return handle_request(request);
        });

    //启动http服务
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
