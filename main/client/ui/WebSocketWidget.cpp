#include "WebSocketWidget.h"
#include <QMessageBox>
#include "MainController.h"

WebSocketWidget::WebSocketWidget(QWebSocket* socket,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WebSocketWidget),m_socket(socket)
{
    ui->setupUi(this);
    if(m_socket==nullptr) m_socket = &gClient;
    connect(m_socket, &QWebSocket::connected, this, &WebSocketWidget::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &WebSocketWidget::disConnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &WebSocketWidget::onTextMessageReceived);
    connect(m_socket, &QWebSocket::binaryMessageReceived,this,&WebSocketWidget::onBinaryMessageReceived);
    connect(&m_reconnectTimer, &QTimer::timeout, this, &WebSocketWidget::tryReconnect);

	ui->checkBox_autoReconnect->setChecked(m_autoReconnect);
	ui->spinBox_reconnectInterval->setValue(m_reconnectInterval / 1000);

    registerFunctions();
    qDebug() << "WebSocket::WebSocket() 构造函数";
}

WebSocketWidget::~WebSocketWidget()
{
	qDebug() << "WebSocket::~WebSocket() 析构函数";
	m_reconnectTimer.stop();
    m_socket->close();
    delete ui;
}

void WebSocketWidget::registerFunctions()
{
    on_pushButton_onConnected_clicked();
}

void WebSocketWidget::on_pushButton_onConnected_clicked()
{
    if (m_socket->state() == QAbstractSocket::UnconnectedState) {
        QString urlStr = ui->lineEdit_url->text().trimmed();// 获取URL并连接
        QUrl url(urlStr);
        if (!url.isValid()) {
            QMessageBox::warning(this, "Error", "Invalid URL!");
            return;
        }
        m_socket->open(url);
        ui->textBrowser_MessageReceived->append("Connecting to " + urlStr + "...");
    }
    else
    {
        // 断开连接
        m_socket->close();
        ui->pushButton_onConnected->setText("连接");
    }
}


void WebSocketWidget::on_pushButton_sendMessage_clicked()
{
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "Error", "Please connect to server first!");
        return;
    }

    QString message = ui->textEdit_MessageSend->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }

    // 发送消息
    m_socket->sendTextMessage(message);

    // 显示发送的消息，加上时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->textBrowser_MessageReceived->append(QString("[%1] Sent:\n %2").arg(timestamp,message));

    // 清空发送框
    ui->textEdit_MessageSend->clear();
}

void WebSocketWidget::tryReconnect()
{
    if (m_autoReconnect && m_socket->state() != QAbstractSocket::ConnectedState) {
		QString urlStr = ui->lineEdit_url->text().trimmed();
        QUrl url(urlStr);
		if (!url.isValid()) {
			QMessageBox::warning(this, "Error", "Invalid URL!");
			return;
		}
        m_socket->open(url);
        ui->textBrowser_MessageReceived->append("尝试重新连接到:" + urlStr + "...");
    }
    else
    {
		m_reconnectTimer.stop();
    }
}

void WebSocketWidget::onConnected()
{
    ui->textBrowser_MessageReceived->append("连接成功");
    ui->pushButton_onConnected->setText("断开");
    QString module_ = south::ShareLib::GetModuleName(south::ModuleName::user);
    m_socket->sendTextMessage(Session::RequestString(1,module_,"login",gSouth.sessiontype_));
	m_reconnectTimer.stop();
}

void WebSocketWidget::disConnected()
{
    emit g_new_message(tr("Disconnect from the server:%1").arg(ui->lineEdit_url->text().trimmed()),LogLevel::Error);
    ui->textBrowser_MessageReceived->append("断开连接");
    ui->pushButton_onConnected->setText("连接");
	if (m_autoReconnect) {
		m_reconnectTimer.start(m_reconnectInterval);
		ui->textBrowser_MessageReceived->append(QString("尝试在%1秒后重连...").arg(m_reconnectInterval / 1000));

	}
}

void WebSocketWidget::onTextMessageReceived(const QString &message)
{
    // 显示接收到的消息，加上时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // 解析消息
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        ui->textBrowser_MessageReceived->append(QString("[%1] Received:\n %2").arg(timestamp,"无效的json数据"));
    }
    ui->textBrowser_MessageReceived->append(QString("[%1] 客户端接收到消息:\n %2").arg(timestamp,message));
    Session session(jsonDoc.object());session.socket = sender();
    for(auto &filter:gSessionFilter){
        if(filter && filter->filter(session)){
            qDebug() <<" 消息被过滤器处理,不再继续默认处理";
            return;
        };
    }
    // 默认处理逻辑
    Result result = gSouth.invoke(session);
    if (!result) {
        qWarning() << "消息处理失败:" << QThread::currentThread() << "[mess	age]" << message;
        ui->textBrowser_MessageReceived->append(QString("[%1]客户端消息处理失败:\n %2").arg(timestamp,result.message));
	}
}

void WebSocketWidget::onBinaryMessageReceived(const QByteArray &message)
{
    qDebug() << "WebSocketWidget::onBinaryMessageReceived" <<message.size();

    QDataStream readStream(message);
    quint8 invoke;
    QByteArray bytes;
    readStream >> invoke;
    readStream >> bytes;
    qDebug() << "WebSocketWidget::onBinaryMessageReceived bytes" <<bytes.size();
    gSouth.handlerBinarySession[south::ModuleName(invoke)](message);
}


void WebSocketWidget::on_checkBox_autoReconnect_stateChanged(int arg1)
{
    m_autoReconnect = (arg1 == Qt::Checked);
    ui->spinBox_reconnectInterval->setEnabled(m_autoReconnect);
}



void WebSocketWidget::on_spinBox_reconnectInterval_valueChanged(int arg1)
{
    m_reconnectInterval = arg1 * 1000; // 转换为毫秒
}

