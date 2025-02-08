#include <QtWebSockets>
//#include "QtWebSockets/qwebsocketserver.h"
//#include "QtWebSockets/qwebsocket.h"
// 内联函数，将字符串转换为JSON对象

#include "WebSocketServer.h"
inline static QJsonObject stringToJson(const QString& jsonString) {
	QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
	if (!jsonDoc.isNull() && jsonDoc.isObject()) {
		return jsonDoc.object();
	}
	else {
		return QJsonObject();
	}
}

// 内联函数，将JSON对象转换为字符串
inline static QString jsonToString(const QJsonObject& jsonObject) {
	QJsonDocument jsonDoc(jsonObject);
	return QString::fromUtf8(jsonDoc.toJson());
}

//响应消息
struct Response {
	int status;          // HTTP状态码风格
	QString function;   //功能描述
	QString message;      // 响应消息
	QString data;     // 响应数据
	// 默认构造函数
	Response() = default;
	// 通过 JSON 字符串初始化
	explicit Response(const QString& jsonString) {
		*this = fromJson(stringToJson(jsonString));
	}
	// 通过 JSON 初始化
	explicit Response(const QJsonObject& json) {
		*this = fromJson(json);
	}
	// 从 JSON 对象初始化
	static Response fromJson(const QJsonObject& json) {
		Response resp;
		resp.status = json.value("status").toInt(200);
		resp.function = json.value("function").toString();
		resp.message = json.value("message").toString("OK");
		resp.data = json.value("data").toString();
		return resp;
	}
	// 从 JSON 对象初始化
	QJsonObject toJson() const {
		QJsonObject obj;
		obj["status"] = status;
		obj["function"] = function;
		obj["message"] = message;
		obj["data"] = data;
		return obj;
	}
	// 转换为 JSON 字符串
	QString toString() const {
		return jsonToString(toJson());
	}
	// 静态方法：返回指定状态码和消息的 JSON 字符串
	static QString toMessageString(int status = 200, QString message = "Invalid Message") {
		Response resp;
		resp.status = status;
		resp.message = message;
		return resp.toString();
	}
};

WebSocketServer::WebSocketServer(quint16 port, QObject* parent)
	:QObject(parent),
	m_pWebSocketServer(new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this))
{
	//m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocket Server"), QWebSocketServer::NonSecureMode, this);
	if (m_pWebSocketServer->listen(QHostAddress::Any, port))
	{
		connect(m_pWebSocketServer, &QWebSocketServer::newConnection, this, &WebSocketServer::onNewConnection);
		connect(m_pWebSocketServer, &QWebSocketServer::closed, this, &WebSocketServer::closed);
	}
	else
	{
		qFatal() << tr("服务启动失败,端口号:%1,请检查端口是否被占用!").arg(port);
	}
}

WebSocketServer::~WebSocketServer()
{
	m_pWebSocketServer->close();
	qDeleteAll(clients);
	qDeleteAll(others);
}

//! [onNewConnection]
void WebSocketServer::onNewConnection()
{
	QWebSocket* pSocket = m_pWebSocketServer->nextPendingConnection();
	qDebug() << QString("Hello from server: %1").arg(pSocket->peerAddress().toString());
	connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::validateDeviceType);
	//connect(pSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
	//connect(pSocket, &QWebSocket::binaryMessageReceived, this, &WebSocketServer::processBinaryMessage);
	connect(pSocket, &QWebSocket::disconnected, this, &WebSocketServer::socketDisconnected);
}
//! [onNewConnection]

void WebSocketServer::validateDeviceType(QString message)
{
	QWebSocket* clientSocket = qobject_cast<QWebSocket*>(sender());
	if (!clientSocket) return;
	QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
	if (doc.isNull()) {
		clientSocket->sendTextMessage(Response::toMessageString(400, "Invalid JSON"));
		return;
	}
	Response resp(doc.object());
	if (resp.function != "DeviceValidation") {
		resp.status = 400;
		resp.message = tr("设备初始化连接验证失败,请求的功能描述错误!");
		clientSocket->sendTextMessage(resp.toString());
		return;
	}
	if (resp.data == "client") {
		clients.insert(clientSocket);
		connect(clientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
		connect(clientSocket, &QWebSocket::disconnected, this, [&]() {
			clients.remove(clientSocket);
			});
	}
	else
	{
		others.insert(clientSocket);
		connect(clientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::processTextMessage);
		connect(clientSocket, &QWebSocket::disconnected, this, [&]() {
			others.remove(clientSocket);
			});
	}
	resp.message = tr("设备类型验证成功,服务端版本是:%1").arg(g_version);
	clientSocket->sendTextMessage(resp.toString());
	// 移除首次验证的槽函数，避免重复验证
	disconnect(clientSocket, &QWebSocket::textMessageReceived, this, &WebSocketServer::validateDeviceType);
}

void WebSocketServer::processTextMessage(QString message)
{
	//sender()是一个QObject成员函数，用于获取发送信号的QObject指针。在信号槽机制中，当一个信号被发射时，sender()函数可以获取发送信号的对象指针。
	QWebSocket* pClient = qobject_cast<QWebSocket*>(sender());
	qDebug() << pClient->peerAddress().toString() << "Message received:" << message;
	//发送信号的对象不是QWebSocket类型），则pClient将为nullptr
	if (pClient) {
		pClient->sendTextMessage(message);
	}
}

void WebSocketServer::processBinaryMessage(QByteArray message)
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

}
