#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QTimer>
#include <QWidget>
#include <QtWebSockets/QWebSocket>
#include "ui_WebSocketWidget.h"
// namespace Ui {
// class WebSocketWidget;
// }

class WebSocketWidget : public QWidget
{
    Q_OBJECT

public:
    Ui::WebSocketWidget *ui;
    //QWebSocket ������ Qt �¼�ѭ�������� Qt ������ʩ����Щ�� QApplication/QCoreApplication ��ʼ��֮ǰ�ǲ����õ�
    explicit WebSocketWidget(QWebSocket* socket = nullptr,QWidget *parent = nullptr);
    ~WebSocketWidget();
	void registerFunctions();

Q_SIGNALS:
    void closed();

public slots:
    void tryReconnect();
    void onConnected();
    void disConnected();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);
private slots:
    void on_pushButton_onConnected_clicked();

    void on_pushButton_sendMessage_clicked();

    void on_checkBox_autoReconnect_stateChanged(int arg1);

    void on_spinBox_reconnectInterval_valueChanged(int arg1);

private:
    QWebSocket* m_socket = nullptr;
    QTimer m_reconnectTimer;
    bool m_autoReconnect{true};
    int m_reconnectInterval{5000};
};

extern QPointer<WebSocketWidget> gWebSocket;
#endif // WEBSOCKET_H
