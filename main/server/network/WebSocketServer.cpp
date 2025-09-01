#include <QtWebSockets>
#include "WebSocketServer.h"

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
		//qCritical() << tr("Fails to WebSocketServer listen port:%1,please check whether the port is occupied!").arg(port);
		//Q_ASSERT(false, "WebSocketServer::listen", "please check whether the port is occupied!"); // Release 模式下不崩溃 使用Q_ASSERT_X
		qFatal() << tr("服务端口:%1 启动失败,请检查是否被占用或者应用程序已经存在!").arg(port);//会直接终止程序
	}
	qDebug() << tr("服务端口:") << port << " 类名称:" << typeid(this).name();
}

/**
 * @brief WebSocketServer 析构函数
 *
 * 关闭WebSocket服务器并清理客户端连接。
 */
WebSocketServer::~WebSocketServer()
{
	qDebug() << "WebSocketServer::~WebSocketServer()";
	m_pWebSocketServer->close();
	qDeleteAll(clients);
	qDeleteAll(others);
}

void WebSocketServer::initialize()
{
	// 通过信号槽在不同线程之间传递消息 使用const不可以使用this的对象,因为父子对象的不是const的
    connect(&gShare, &share::Shared::sigSent, this, &WebSocketServer::handleMessageSent);
	connect(&gShare, &share::Shared::sigSentBinary, this, &WebSocketServer::handleBinarySent);
	connect(&gLog, &Logger::new_message, this, &WebSocketServer::handleLogMessageSent);
}

void WebSocketServer::SentMessageToClients(const QString& message) {
	static QQueue<QString> cachedMessages; //QQueue是一个先进先出(FIFO - First In First Out)的队列数据结构
	if(!message.isEmpty()){
		cachedMessages.enqueue(message); // 将新消息添加到队列尾部
	}
	static const int maxCachedMessages = 100;// 设置队列最大容量，防止内存溢出
	while (cachedMessages.size() > maxCachedMessages) {
		cachedMessages.dequeue();// 当队列超过最大容量时，从队首移除旧消息
	}
	if (!clients.isEmpty()) {
		// 取出并发送所有消息
		while (!cachedMessages.isEmpty()) {
			QString msg = cachedMessages.dequeue(); // 取出并移除队首消息
			for (auto client : clients) {
				client->sendTextMessage(msg); // 发送给每个客户端
			}
		}
	}
}


void WebSocketServer::handleMessageSent(const QString& message, QObject* wsclient)
{
	qDebug() <<"#Sent:size is"<< message.size()<< QThread::currentThread();
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

void WebSocketServer::handleBinarySent(const QByteArray& message, QObject* wsclient) {
	qDebug() << "#Sent:size is" << message.size() << QThread::currentThread();
	QPointer<QWebSocket> socket = qobject_cast<QWebSocket*>(wsclient);
	//多线程下 加锁
	static QMutex mutex_socket;
	QMutexLocker locker(&mutex_socket);
	if (message.isEmpty()) return;
	if (socket) { socket->sendBinaryMessage(message); } else {
		for (QPointer<QWebSocket> client : clients) {
			client->sendBinaryMessage(message);
		}
	}
}

void WebSocketServer::handleLogMessageSent(const QString& message, LogLevel level) {
    //qDebug() << "处理发送到日志消息handleLogMessageSent:" << message << double(level);
	// 创建JSON数组并添加消息内容和日志级别
	//QJsonArray array;array.append(message);array.append(double(level));
	QJsonArray array = { message, double(level) };
	QString msg = Session::RequestString(0, sModuleUser, "onShowMessage", array);
	SentMessageToClients(msg);
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
	//预留先进行设备验证,再进行信号连接
	qDebug() << "verify_device_type:" << message;
	QWebSocket* newSocket = qobject_cast<QWebSocket*>(sender());
	if (!newSocket) return;
	QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull()) {//向客户端发送错误信息
		newSocket->sendTextMessage(Session::RequestString(1, "user","error",tr("无效的JOSN数据:%1").arg(message)));
		return;
	}
	Session resp(doc.object());
	if (resp.method != "login") {
		QString error_message = tr("请先登录,请求方法: %1,然后进行其他操作").arg(resp.method);
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
		foreach( auto mod, gShare.GetHandlerList()) {//模块初始化信息,界面信息初始化
			newSocket->sendTextMessage(Session::RequestString(mod, "onEnableChanged", QJsonArray{ true }));
		}
		SentMessageToClients();//发送之前缓存信息
		break;
	default:
		others.insert(newSocket);
		connect(newSocket, &QWebSocket::disconnected, this, [&]() {
			others.remove(newSocket);
		});
		deviceType = SessionType::Other;
		SentMessageToClients(Session::RequestString(sModuleUser, "login_new_session", QJsonArray{ double(deviceType)}));
		break;
	}
	// 移除首次验证的槽函数，避免重复验证
	disconnect(newSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::verify_device_type);
    // 连接消息处理槽函数,目前统一处理
	connect(newSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::handle_text_message);
	//返回消息,验证设备版本号是否一致
	//handleMessageSent()
	newSocket->sendTextMessage(Session::RequestString(sModuleUser, "onSignIn", QJsonArray{ gShare.info }));
}


void WebSocketServer::handle_text_message(const QString& message)
{
	//sender()是一个QObject成员函数，用于获取发送信号的QObject指针。在信号槽机制中，当一个信号被发射时，sender()函数可以获取发送信号的对象指针。
	//如果 pClient 是 QWebSocket 对象，并且它的父对象是 QWebSocketServer，那么它的生命周期将由 QWebSocketServer 管理，不需要手动释放。
	//WebSocketPtr pClient = WebSocketPtr(qobject_cast<QWebSocket*>(sender()));
	QPointer<QWebSocket> pClient = qobject_cast<QWebSocket*>(sender());
	qDebug() << "#[SendTo]" << pClient->peerAddress().toString() << "[Message]" << message.size() << QThread::currentThread();;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
	if (jsonDoc.isNull() || !jsonDoc.isObject()) {//向客户端发送错误信息
		pClient->sendTextMessage(Session::RequestString(1, "user", "error", tr("无效的JOSN数据:%1").arg(message.size())));
	}
	//Result result = gController.invoke(jsonDoc.object(), pClient);
	Result result = share::Shared::instance().invoke(jsonDoc.object(), sender());
    if (!result) {
		qWarning() << "消息处理失败:" << QThread::currentThread() << "[message]" << message;
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
	qDebug() << pClient->peerAddress().toString() << "Binary Message received";
	//pClient->sendBinaryMessage(message);
	QDataStream in(message);
	//in.setVersion(QDataStream::Qt_5_15);
	quint8 invoke;
	in >> invoke;
	QByteArray data;
	in >> data;
	qDebug() << "invoke:" << invoke << "size:" << data.size();
	gShare.handlerBinarySession[share::ModuleName(invoke)](data);
}

void WebSocketServer::on_socket_disconnected()
{
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
	qInfo() << "连接断开:" << pClient->peerAddress().toString();
	//others.remove(pClient);
}
