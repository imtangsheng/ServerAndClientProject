#include "stdafx.h"
#include "WebSocketServer.h"
#include <QtWebSockets>


WebSocketServer::WebSocketServer(quint16 port, QObject* parent)
	:QObject(parent),
	m_pWebSocketServer(new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this))
{
	//m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);
	if (m_pWebSocketServer->listen(QHostAddress::Any, port)) {
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
		connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WebSocketServer::closed);
	}
	else {
		qFatal() << tr("服务启动失败,端口号:%1,请检查端口是否被占用!").arg(port);
	}

	// 创建处理器实例 确保 parent 参数是非 const 的 QObject 指针是
	gTrolleyController = (new TrolleyController((this)));
	gFaroController = (new FaroController(this));
	gCameraController = (new CameraController(this));
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

void WebSocketServer::initialize() const
{
	// 通过信号槽在不同线程之间传递消息 使用const不可以使用this的对象,因为父子对象的不是const的
	connect(&gController, &Controller::sigSend, this, &WebSocketServer::sendMessage);
}

void WebSocketServer::sendMessage(const QString& message, QWebSocket* socket)
{
	//多线程下 加锁
	static QMutex mutex_socket;
	QMutexLocker locker(&mutex_socket);
	if (message.isEmpty()) return;
	if (socket) {socket->sendTextMessage(message);}
	else {
		for (QWebSocket* client : clients) {
			client->sendTextMessage(message);
		}
	}
}

/**
 * @brief 处理新连接
 *
 * 当有新的WebSocket连接时调用此函数。
 */
void WebSocketServer::onNewConnection()
{
	QWebSocket* pSocket = m_pWebSocketServer->nextPendingConnection();
	qDebug() << QString("New Connection from: %1").arg(pSocket->peerAddress().toString());
	//connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::validateDeviceType);
	connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
	//connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::processBinaryMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &WebSocketServer::socketDisconnected);
	others.insert(pSocket);
}

/**
 * @brief 验证设备类型
 *
 * @param message 接收到的消息
 */
void WebSocketServer::validateDeviceType(const QString& message)
{
	//TODO 预留先进行设备验证,再进行信号连接
	QWebSocket* clientSocket = qobject_cast<QWebSocket*>(sender());
	if (!clientSocket) return;
	QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
	if (doc.isNull()) {
		clientSocket->sendTextMessage(jsonToString({ {"Invalid JSON",400} }));
		return;
	}
	//Request resp(doc.object());
	//if (resp.method != "DeviceValidation") {
		//resp.message = tr("设备初始化连接验证失败,请求的功能描述错误!");
		//clientSocket->sendTextMessage(resp.toString());
		//return;
	//}
	//if (resp.data == "client") {
	clients.insert(clientSocket);
	connect(clientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
	connect(clientSocket, &QWebSocket::disconnected, this, [&]() {
		clients.remove(clientSocket);
		});
	//}
	//else
	//{
	//	others.insert(clientSocket);
	//	connect(clientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
	//	connect(clientSocket, &QWebSocket::disconnected, this, [&]() {
	//		others.remove(clientSocket);
	//		});
	//}
	//resp.message = tr("设备类型验证成功,服务端版本是:%1").arg(g_version);
	//clientSocket->sendTextMessage(resp.toString());
	// 移除首次验证的槽函数，避免重复验证
	disconnect(clientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::validateDeviceType);
}

/**
 * @brief 处理文本消息
 *
 * @param message 接收到的文本消息
 */
void WebSocketServer::processTextMessage(const QString& message)
{
	//sender()是一个QObject成员函数，用于获取发送信号的QObject指针。在信号槽机制中，当一个信号被发射时，sender()函数可以获取发送信号的对象指针。
	//如果 pClient 是 QWebSocket 对象，并且它的父对象是 QWebSocketServer，那么它的生命周期将由 QWebSocketServer 管理，不需要手动释放。
	//WebSocketPtr pClient = WebSocketPtr(qobject_cast<QWebSocket*>(sender()));
	SocketPtr pClient = qobject_cast<QWebSocket*>(sender());
	qDebug() << "#Received:" << pClient->peerAddress().toString() << "[Message]" << message << QThread::currentThread();;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
	if (jsonDoc.isNull() || !jsonDoc.isObject()) {
		pClient->sendTextMessage(jsonToString({ {"id", -1}, {"code", -1}, {"message",  tr("处理失败,传入的数据不是json格式:%1").arg(message)} }));
	}
	Result result = gController.invoke(jsonDoc.object(), pClient);
	if (!result.success) {
		qWarning() << "消息处理失败:" << QThread::currentThread() << "[mess	age]" << message;
		pClient->sendTextMessage(result.message);
	}

}

/**
 * @brief 处理二进制消息
 *
 * @param message 接收到的文本消息
 */
void WebSocketServer::processBinaryMessage(const QByteArray& message)
{
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
	qDebug() << pClient->peerAddress().toString() << "Binary Message received:" << message;
	if (pClient) {
		pClient->sendBinaryMessage(message);
	}
}

void WebSocketServer::socketDisconnected()
{
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
	qInfo() << "socketDisconnected:" << pClient;
	others.remove(pClient);
}
