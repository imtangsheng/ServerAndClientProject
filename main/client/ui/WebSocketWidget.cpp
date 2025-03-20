#include "WebSocketWidget.h"
#include <QMessageBox>

WebSocketWidget::WebSocketWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WebSocketWidget)
{
    ui->setupUi(this);
    connect(&gClient, &QWebSocket::connected, this, &WebSocketWidget::onConnected);
    connect(&gClient, &QWebSocket::disconnected, this, &WebSocketWidget::disConnected);
    connect(&gClient, &QWebSocket::textMessageReceived, this, &WebSocketWidget::onTextMessageReceived);
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
    gClient.close();
    delete ui;
}

void WebSocketWidget::registerFunctions()
{
    on_pushButton_onConnected_clicked();
}

void WebSocketWidget::on_pushButton_onConnected_clicked()
{
    if (gClient.state() == QAbstractSocket::UnconnectedState) {
        QString urlStr = ui->lineEdit_url->text().trimmed();// 获取URL并连接
        QUrl url(urlStr);
        if (!url.isValid()) {
            QMessageBox::warning(this, "Error", "Invalid URL!");
            return;
        }
        gClient.open(url);
        ui->textBrowser_MessageReceived->append("Connecting to " + urlStr + "...");
    }
    else
    {
        // 断开连接
        gClient.close();
        ui->pushButton_onConnected->setText("连接");
    }
}


void WebSocketWidget::on_pushButton_sendMessage_clicked()
{
    if (gClient.state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "Error", "Please connect to server first!");
        return;
    }

    QString message = ui->textEdit_MessageSend->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }

    // 发送消息
    gClient.sendTextMessage(message);

    // 显示发送的消息，加上时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ui->textBrowser_MessageReceived->append(QString("[%1] Sent:\n %2").arg(timestamp,message));

    // 清空发送框
    ui->textEdit_MessageSend->clear();
}

void WebSocketWidget::tryReconnect()
{
    if (m_autoReconnect && gClient.state() != QAbstractSocket::ConnectedState) {
		QString urlStr = ui->lineEdit_url->text().trimmed();
        QUrl url(urlStr);
		if (!url.isValid()) {
			QMessageBox::warning(this, "Error", "Invalid URL!");
			return;
		}
        gClient.open(url);
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
    gClient.sendTextMessage(Session::RequestString(1,"user","login",gSouth.type));
	m_reconnectTimer.stop();
}

void WebSocketWidget::disConnected()
{
    ui->textBrowser_MessageReceived->append("断开连接");
    ui->pushButton_onConnected->setText("连接");
	if (m_autoReconnect) {
		m_reconnectTimer.start(m_reconnectInterval);
		ui->textBrowser_MessageReceived->append(QString("尝试在%1秒后重连...").arg(m_reconnectInterval / 1000));

	}
}

void WebSocketWidget::onTextMessageReceived(QString message)
{
    // 显示接收到的消息，加上时间戳
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    // 解析消息
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        ui->textBrowser_MessageReceived->append(QString("[%1] Received:\n %2").arg(timestamp,"无效的json数据"));
    }
    ui->textBrowser_MessageReceived->append(QString("[%1] 客户端接收到消息:\n %2").arg(timestamp,message));
    //Session session(jsonDoc.object());
    Result result = south::ShareLib::instance().invoke(jsonDoc.object(), sender());
    if (!result) {
        qWarning() << "消息处理失败:" << QThread::currentThread() << "[mess	age]" << message;
        ui->textBrowser_MessageReceived->append(QString("[%1]客户端消息处理失败:\n %2").arg(timestamp,result.message));
	}
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

