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
* Qt ֧�� GET��POST �������� HTTP ������body() �����Ե��ã��Ҳ����׳��쳣
* ���������ɺ��� 1)ȫ�־�̬,2)�޲��� ������
* �󶨵���Ա���� Lambda��ʽ
QJsonObject(*)(const QHttpServerRequest&)���˴�Ϊ����ָ�� void (*funcPtr)() = myFunction;  // �Ϸ�����ʽת��
void (*funcPtr)() = &myFunction; // �Ϸ�����ʽȡ��ַ
*/
static QJsonObject RestartApp() {
    //ʹ��ϵͳ�ض���������������ƽ̨��
    // �ӳ�����,�������������
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
    /*����GET����*/
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

    server.route("/get", QHttpServerRequest::Method::Get,
        [this](const QHttpServerRequest &request) {
            return handle_request(request);
        });

    //����http����
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
