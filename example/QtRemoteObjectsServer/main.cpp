#include <QCoreApplication>
#include <QtRemoteObjects>
#include <QDebug>
#include <QWebSocketServer>
#include <QWebSocket>
#include "websocketiodevice.h"

class MyRemoteObject : public QObject
{
	Q_OBJECT

public:
	MyRemoteObject(QObject* parent = nullptr) : QObject(parent) {}

signals:
	void send1(const QString& message); // 服务端主动触发的信号
public slots:
	QString sayHello(const QString& name) {
		qDebug() << "Received request from client: " << name;
		return QString("Hello, %1!").arg(name);
	}
	void set(int name) {

		qDebug() << "Received request from client: " << name;
		name = 1;
		emit send1("send tets");
	}
};
/*
* QtRO 不支持 WebSocket 作为通信协议
* Qt Remote Objects 目前不原生支持 WebSocket（ws://）作为传输协议。默认情况下，它支持：
* 本地 socket (local:)
* TCP socket (tcp:)
* QIODevice 传输
*/
int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	// 创建远程对象
	MyRemoteObject remoteObject;

	// 创建 QRemoteObjectHost 并注册远程对象
	//QRemoteObjectHost host(QUrl(QStringLiteral("local:replica")));//本地协议（Local Socket）
	//QRemoteObjectHost host(QUrl("tcp://0.0.0.0:12345"));//TCP 协议
	//QRemoteObjectHost host(QUrl("ipc:///tmp/myinterface"));//IPC（进程间通信）：URL 格式：ipc://<路径> 说明：使用本地文件路径作为通信通道。
	//host.enableRemoting(&remoteObject, "MyRemoteObject");

	// 启动WebSocket服务器
	QWebSocketServer server(QStringLiteral("QtRO WebSocket Server"), QWebSocketServer::NonSecureMode, &a);
	if (!server.listen(QHostAddress::Any, 12345)) {
		qCritical() << "Failed to start WebSocket server.";
		return -1;
	}

	qDebug() << "WebSocket server started..."<< server.serverAddress().toString();
	// 创建 QtRO 远程对象主机
	QRemoteObjectHost host;
	host.setHostUrl(server.serverAddress().toString(), QRemoteObjectHost::AllowExternalRegistration);

	// 使用QRemoteObjectHost启用WebSocket通信
	host.enableRemoting(&remoteObject, "MyRemoteObject");

	qDebug() << "Server is running...";
	QObject::connect(&server, &QWebSocketServer::newConnection, &host, [&host, &server] {
		while (auto conn = server.nextPendingConnection()) {
			QObject::connect(conn, &QWebSocket::disconnected, conn, &QWebSocket::deleteLater);
			QObject::connect(conn, &QWebSocket::errorOccurred, conn, &QWebSocket::deleteLater);
			auto ioDevice = new WebSocketIoDevice(conn);
			QObject::connect(conn, &QWebSocket::destroyed, ioDevice, &WebSocketIoDevice::deleteLater);
			host.addHostSideConnection(ioDevice);
		}
		});

	return a.exec();
}

#include "main.moc"
