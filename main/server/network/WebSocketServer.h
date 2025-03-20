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
    /**
     * @brief ����������
     *
     * �����µ�WebSocket����ʱ���ô˺�����
     */
    void accept_new_connected();
    /**
     * @brief ��֤�豸����
     *
     * @param message ���յ�����Ϣ
     */
    void verify_device_type(const QString& message);
    /**
     * @brief �����ı���Ϣ
     *
     * @param message ���յ����ı���Ϣ
     */
    void handle_text_message(const QString& message);
    void handle_binary_message(const QByteArray& message);
    void on_socket_disconnected();

private:
    QWebSocketServer* m_pWebSocketServer;
};