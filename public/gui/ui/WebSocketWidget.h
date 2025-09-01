#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <QTimer>
#include <QWidget>
#include <QtWebSockets/QWebSocket>
#include "ui_WebSocketWidget.h"

/*���ɿ�������,Ҳ����������qApp�ȳ�ʼ��
 * Ĭ�ϵ�ȫ������һ������,��mainwindow.ui�еľֲ����ɵ�
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
    //QWebSocket ������ Qt �¼�ѭ�������� Qt ������ʩ����Щ�� QApplication/QCoreApplication ��ʼ��֮ǰ�ǲ����õ�
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
