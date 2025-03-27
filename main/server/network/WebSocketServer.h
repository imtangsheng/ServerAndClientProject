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
    void handleMessageSent(const QString& message, QObject* wsclient);
Q_SIGNALS:
    void closed();

private Q_SLOTS:
    /**
     * @brief 处理新连接
     *
     * 当有新的WebSocket连接时调用此函数。
     */
    void accept_new_connected();
    /**
     * @brief 验证设备类型
     *
     * @param message 接收到的消息
     */
    void verify_device_type(const QString& message);
    /**
     * @brief 处理文本消息
     *
     * @param message 接收到的文本消息
     */
    void handle_text_message(const QString& message);
    void handle_binary_message(const QByteArray& message);
    void on_socket_disconnected();

private:
    QWebSocketServer* m_pWebSocketServer;
};