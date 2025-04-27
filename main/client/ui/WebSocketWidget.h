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
    //QWebSocket 依赖于 Qt 事件循环和其他 Qt 基础设施，这些在 QApplication/QCoreApplication 初始化之前是不可用的
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
