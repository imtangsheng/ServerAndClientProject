#pragma once
#include <QObject>
#include <QSet>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

inline QString g_version = "1.0.0";

class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(quint16 port, QObject* parent = nullptr);
    ~WebSocketServer();
    QSet<QWebSocket*> clients;
    QSet<QWebSocket*> others;

Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection();
    void validateDeviceType(QString message);
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();

private:
    QWebSocketServer* m_pWebSocketServer;
};