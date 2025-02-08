#include "WebSocketClient.h"

WebSocketClient::WebSocketClient(const QUrl& url, QObject* parent) :
	QObject(parent)
{
	connect(&m_webSocket, &QWebSocket::connected, this, &WebSocketClient::onConnected);
	connect(&m_webSocket, &QWebSocket::disconnected, this, &WebSocketClient::closed);
	connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);
	m_webSocket.open(url);
}

void WebSocketClient::onConnected()
{
	qDebug() << "WebSocket connected";
}

void WebSocketClient::onTextMessageReceived(QString message)
{
	QWebSocket* pServer = qobject_cast<QWebSocket*>(sender());
	qDebug() << pServer->peerAddress().toString() << "Message received:" << message;
}
