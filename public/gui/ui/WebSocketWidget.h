#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QTimer>
#include <QWidget>
#include <QtWebSockets/QWebSocket>
#include "ui_WebSocketWidget.h"

/*不可拷贝复制,也不可以先于qApp先初始化
 * 默认的全局网络一个变量,是mainwindow.ui中的局部集成的
*/
#define gClient GetWebSocketClient()
inline QSharedPointer<QWebSocket> gSocket{nullptr};
inline QWebSocket& GetWebSocketClient(){
    static QWebSocket socketClient;
    return socketClient;
}

class WebSocketWidget : public QWidget
{
    Q_OBJECT

public:
    Ui::WebSocketWidget *ui;
    //QWebSocket 依赖于 Qt 事件循环和其他 Qt 基础设施，这些在 QApplication/QCoreApplication 初始化之前是不可用的
    explicit WebSocketWidget(QWebSocket* newsocket,QWidget *parent = nullptr);
    ~WebSocketWidget();

    QPointer<QWebSocket> socket;
    QString url;
    bool isAutoReconnect;
    int reconnectInterval;
    QTimer reconnectTimer;
public slots:
    void tryReconnect();
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(QByteArray message);
private slots:
    void on_pushButton_sendMessage_clicked();

private:


Q_SIGNALS:
    void closed();
};

#endif // WEBSOCKET_H
