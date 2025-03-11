#pragma once
#include <QObject>
#include <QSet>
#include <QMutex>

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

class WebSocketServer : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketServer(quint16 port, QObject* parent = nullptr);
    ~WebSocketServer();

    void initialize();
    QMutex mutex;
    QSet<QWebSocket*> clients;
    QSet<QWebSocket*> others;

public slots:
    void sent_message(const QString& message, QObject* wsclient);
Q_SIGNALS:
    void closed();

private Q_SLOTS:
    void onNewConnection();
    void validateDeviceType(const QString& message);
    void processTextMessage(const QString& message);
    void processBinaryMessage(const QByteArray& message);
    void socketDisconnected();

private:
    QWebSocketServer* m_pWebSocketServer;
};