#include <QtWebSockets>
#include "manager/ManagerPlugin.h"
const QString g_version = "v1.0.0";

WebSocketServer::WebSocketServer(quint16 port, QObject* parent)
	:QObject(parent),
	m_pWebSocketServer(new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this))
{
	//m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);
	if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WebSocketServer::accept_new_connected);
		connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WebSocketServer::closed);
	}
	else {
		qFatal() << tr("服务启动失败,端口号:%1,请检查端口是否被占用!").arg(port);//会直接终止程序
	}

	// 创建处理器实例 确保 parent 参数是非 const 的 QObject 指针是
	//gTrolleyController = (new TrolleyController((this)));
	//gFaroController = (new FaroController(this));
	gManagerPlugin = (new ManagerPlugin(this));
}

/**
 * @brief WebSocketServer 析构函数
 *
 * 关闭WebSocket服务器并清理客户端连接。
 */
WebSocketServer::~WebSocketServer()
{
	m_pWebSocketServer->close();
	qDeleteAll(clients);
	qDeleteAll(others);
}

void WebSocketServer::initialize()
{
	//qDebug() << "#测试:" << &gController << QThread::currentThreadId();
	// 通过信号槽在不同线程之间传递消息 使用const不可以使用this的对象,因为父子对象的不是const的
	//connect(&gController, &Controller::sigSend, this, &WebSocketServer::sent_message);
    connect(&gSouth, &south::ShareLib::sent, this, &WebSocketServer::sent_message);
	if (gManagerPlugin->PluginsScan()) {
		for (QString& pluginName:gManagerPlugin->pluginsAvailable)
		{	
			//是否是无效插件
			if (gManagerPlugin->pluginsInvalid.contains(pluginName)) continue;
			gManagerPlugin->PluginLoad(pluginName);
		}
	}
}

void WebSocketServer::sent_message(const QString& message, QObject* wsclient)
{
	QPointer<QWebSocket> socket = qobject_cast<QWebSocket*>(wsclient);
	//多线程下 加锁
	static QMutex mutex_socket;
	QMutexLocker locker(&mutex_socket);
	if (message.isEmpty()) return;
	if (socket) {socket->sendTextMessage(message);}
	else {
		for (QPointer<QWebSocket> client : clients) {
			client->sendTextMessage(message);
		}
	}
}


void WebSocketServer::accept_new_connected()
{
	QWebSocket* pSocket = m_pWebSocketServer->nextPendingConnection();
	qDebug() << QString("New Connection from: %1").arg(pSocket->peerAddress().toString());
	connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::verify_device_type);
	//connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::handle_text_message);
	//connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::handle_binary_message);
	connect(pSocket, &QWebSocket::disconnected, this, &WebSocketServer::on_socket_disconnected);
	//others.insert(pSocket);
}


void WebSocketServer::verify_device_type(const QString& message)
{
	//TODO 预留先进行设备验证,再进行信号连接
	QWebSocket* newSocket = qobject_cast<QWebSocket*>(sender());
	if (!newSocket) return;
	QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull()) {//向客户端发送错误信息
		newSocket->sendTextMessage(Session::RequestString(1, "user","error",tr("发送了无效的josn数据:%1").arg(message)));
		return;
	}
	Session resp(doc.object());
	if (resp.method != "login") {
		QString error_message = tr("请先进行设备初始化连接验证,请先登录,登录验证错误:%1").arg(resp.method);
		newSocket->sendTextMessage(resp.ErrorString(1, error_message));
		return;
	}
	// 设备类型验证和分类
	SessionType deviceType = SessionType(resp.params.toInt(0));
	switch (deviceType) {
	case SessionType::Client:
		clients.insert(newSocket);
		connect(newSocket, &QWebSocket::disconnected, this, [&]() {
			clients.remove(newSocket);
			});
        deviceType = SessionType::Client;
		break;
	default:
		others.insert(newSocket);
		connect(newSocket, &QWebSocket::disconnected, this, [&]() {
			others.remove(newSocket);
		});
		deviceType = SessionType::Other;
		break;
	}
	// 移除首次验证的槽函数，避免重复验证
	disconnect(newSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::verify_device_type);
    // 连接消息处理槽函数,目前统一处理
	connect(newSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::handle_text_message);
	resp.result = int(deviceType);
	newSocket->sendTextMessage(resp.ResponseString(tr("设备类型验证成功, 服务端版本是:%1").arg(g_version)));
}


void WebSocketServer::handle_text_message(const QString& message)
{
	//sender()是一个QObject成员函数，用于获取发送信号的QObject指针。在信号槽机制中，当一个信号被发射时，sender()函数可以获取发送信号的对象指针。
	//如果 pClient 是 QWebSocket 对象，并且它的父对象是 QWebSocketServer，那么它的生命周期将由 QWebSocketServer 管理，不需要手动释放。
	//WebSocketPtr pClient = WebSocketPtr(qobject_cast<QWebSocket*>(sender()));
	QPointer<QWebSocket> pClient = qobject_cast<QWebSocket*>(sender());
	qDebug() << "#Received:" << pClient->peerAddress().toString() << "[Message]" << message << QThread::currentThread();;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
	if (jsonDoc.isNull() || !jsonDoc.isObject()) {//向客户端发送错误信息
		pClient->sendTextMessage(Session::RequestString(1, "user", "error", tr("发送了无效的josn数据:%1").arg(message)));
	}
	//Result result = gController.invoke(jsonDoc.object(), pClient);
	Result result = south::ShareLib::instance().invoke(jsonDoc.object(), sender());
    if (!result) {
		qWarning() << "消息处理失败:" << QThread::currentThread() << "[mess	age]" << message;
		pClient->sendTextMessage(result.message);
	}

}

/**
 * @brief 处理二进制消息
 *
 * @param message 接收到的文本消息
 */
void WebSocketServer::handle_binary_message(const QByteArray& message)
{
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
	qDebug() << pClient->peerAddress().toString() << "Binary Message received:" << message;
	if (pClient) {
		pClient->sendBinaryMessage(message);
	}
}

void WebSocketServer::on_socket_disconnected()
{
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
	qInfo() << "on_socket_disconnected:" << pClient;
	//others.remove(pClient);
}
