#include "WebSocketWidget.h"
#include <QMessageBox>

WebSocketWidget::WebSocketWidget(QWebSocket* newsocket,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WebSocketWidget),socket(newsocket)
{
    ui->setupUi(this);
    if(socket==nullptr) socket = new QWebSocket();
    // connect(socket, &QWebSocket::connected, this, &WebSocketWidget::onConnected);
    // connect(socket, &QWebSocket::disconnected, this, &WebSocketWidget::disConnected);
    connect(socket, &QWebSocket::textMessageReceived, this, &WebSocketWidget::onTextMessageReceived);
    connect(socket, &QWebSocket::binaryMessageReceived,this,&WebSocketWidget::onBinaryMessageReceived);
    connect(&reconnectTimer, &QTimer::timeout, this, &WebSocketWidget::tryReconnect);
    qDebug() << "WebSocket::WebSocket() 构造函数";
}

WebSocketWidget::~WebSocketWidget()
{
	qDebug() << "WebSocket::~WebSocket() 析构函数";
    reconnectTimer.stop();
    socket->close();
    delete ui;
}

void WebSocketWidget::on_pushButton_sendMessage_clicked()
{
    if (socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "Error", "Please connect to server first!");
        return;
    }

    QString message = ui->textEdit_MessageSend->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }

    // 发送消息
    socket->sendTextMessage(message);

    // 显示发送的消息，加上时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->textBrowser_MessageReceived->append(QString("[%1] Sent:\n %2").arg(timestamp,message));

    // 清空发送框
    ui->textEdit_MessageSend->clear();
}

void WebSocketWidget::tryReconnect()
{
    qDebug() << tr("尝试重新连接到:") << url;
    if (isAutoReconnect && socket->state() != QAbstractSocket::ConnectedState) {
        socket->open(url);
        ui->textBrowser_MessageReceived->append(tr("尝试重新连接到:") + url + "...");
    }
    else
    {
        reconnectTimer.stop();
    }
}


void WebSocketWidget::onTextMessageReceived(const QString &message)
{
    // 显示接收到的消息，加上时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // 解析消息
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        ui->textBrowser_MessageReceived->append(tr("[%1] Received:\n %2").arg(timestamp,"无效的json数据"));
    }
    ui->textBrowser_MessageReceived->append(tr("[%1] 客户端接收到消息:\n %2").arg(timestamp,message));
    Session session(jsonDoc.object());session.socket = sender();
    for(auto &filter:gSessionFilter){
        if(filter && filter->filter(session)){
            // qDebug() <<" 消息被过滤器处理,不再继续默认处理";
            return;
        };
    }
    // 默认处理逻辑
    Result result = gShare.invoke(session);
    if (!result) {
        qWarning() << "消息处理失败:" << QThread::currentThread() << "[mess	age]" << message;
        ui->textBrowser_MessageReceived->append(tr("[%1]客户端消息处理失败:\n %2").arg(timestamp,result.message));
	}


}

/**使用值传递 Qt 会对 QByteArray 进行写时复制（copy-on-write）优化，所以值传递的开销通常不会很大
 * 数据小于10MB，使用默认的COW机制即可 (只有在修改时才会开始复制,否则只是引用)
 * 10MB-100MB，考虑使用QSharedPointer
 * 100MB，考虑使用内存映射或流式处理
 **/
void WebSocketWidget::onBinaryMessageReceived(const QByteArray& message)
{
    QByteArray& bytes = const_cast<QByteArray&>(message);
    quint8 invoke = bytes[0];
    bytes.remove(0, 1);  // 移除第一个字节
    qDebug() << "WebSocketWidget::onBinaryMessageReceived bytes" << bytes.size();
    gShare.handlerBinarySession[share::ModuleName(invoke)](bytes);
}
