#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "ui/WebSocketWidget.h"

QPointer<WebSocketWidget> gWebSocket;
#include"dialog/HttpDialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    gSouth.RegisterHandler(module_,this);
    /*模块 信号连接*/
    connect(&gClient, &QWebSocket::stateChanged, this, &MainWindow::onStateChanged);

    gWebSocket = new WebSocketWidget();
    int index = ui->stackedWidget_DeviceManager_items_screen->addWidget(gWebSocket->ui->widget_DeviceManager);
    ui->verticalLayout_DeviceManager_items_name->insertWidget(index,gWebSocket->ui->ButtonMenu_DeviceManager);
    connect(gWebSocket->ui->ButtonMenu_DeviceManager,&QRadioButton::clicked, this, [&]{
        ui->stackedWidget_DeviceManager_items_screen->setCurrentWidget(gWebSocket->ui->widget_DeviceManager);
    });

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::login_verify(double type, const QString &version)
{
    if(int(type) != gSouth.sessiontype_){
        show_message(tr("the device type does not match and the server is %1 a client is %2 ").arg(type).arg(gSouth.sessiontype_),LogLevel::Warning);
    }

    if(version != gSouth.version){
        show_message(tr("the device version does not match and the server is %1 a client is %2").arg(version,gSouth.version),LogLevel::Warning);
    }
    show_message(tr("the server connection is successful"),LogLevel::Info);

    //获取设备状态
    // gClient.sendTextMessage(Session::RequestString(11,module_,"onDeviceStateChanged",true));
}

void MainWindow::onStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "WebSocket state changed:" << state;
    switch (state) {
    case QAbstractSocket::UnconnectedState://未连接状态（Unconnected State）：表示套接字未连接到任何远程主机,可能是应用程序启动时的初始状态,或者连接被显式关闭
        onNetworkStateShow(tr("The socket is not connected"));
        break;
    case QAbstractSocket::HostLookupState://主机查找状态（Host Lookup State）：在这个阶段,套接字正在执行 DNS 查找,以解析连接请求中提供的主机名
        onNetworkStateShow(tr("The socket is performing a host name lookup"),true);
        break;
    case QAbstractSocket::ConnectingState://连接状态（Connecting State）：套接字已经启动连接过程,正在与远程主机建立连接
        onNetworkStateShow(tr("The socket has started establishing a connection"),true);
        break;
    case QAbstractSocket::ConnectedState://已连接状态（Connected State）：这是理想状态,成功建立连接,套接字准备进行双向通信
        onNetworkStateShow(tr("The scocket has already established a connection"),true,true);
        break;
    case QAbstractSocket::BoundState://绑定状态（Bound State）：套接字已绑定到特定的地址和端口,通常用于服务器端 WebSocket 实现
        onNetworkStateShow(tr("The socket is bound to an address and port"),true);
        break;
    case QAbstractSocket::ClosingState://关闭状态（Closing State）：套接字正在关闭连接,可能是由用户主动断开,或者由于错误条件引起
        onNetworkStateShow(tr("The socket is about to close"));
        break;
    default:
        onNetworkStateShow(tr("Unknown state"));
        break;
    }
}

void MainWindow::onNetworkStateShow(const QString &msg, const bool &isConnecting, const bool &isConnected)
{
    qDebug() << "msg" << msg <<"isConnecting"<<isConnecting << "isConnected"<<isConnected;
}

void MainWindow::show_message(const QString &message, LogLevel level)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString levelStr;
    QColor color;
    switch (level) {
    case LogLevel::Error:
        levelStr = tr("[Error]");
        color = Qt::red;
        // ui->toolButton_alram->setAlarmState(AlarmToolButton::Error);
        break;
    case LogLevel::Warning:
        levelStr = tr("[Warning]");
        color = Qt::yellow;
        // ui->toolButton_alram->setAlarmState(AlarmToolButton::Warning);
        break;
    default:
        color = QColor(128, 128, 128);// 灰色
        break;
    }
    qDebug() << int(level) <<message<<color.name();
    // ui->plainTextEdit_log->appendPlainText(QString("[%1] %2 %3").arg(timestamp).arg(levelStr).arg(message));
    ui->plainTextEdit_log->appendHtml(QString("<span style='color: %1'>[%2] %3 %4</span>").arg(color.name(),timestamp,levelStr,message));
}

void MainWindow::on_pushButton_language_switch_clicked()
{
    if(g_language == en_US)
    {
        qApp->removeTranslator(&g_translator);
        g_language = zh_CN;
        qDebug()<<"当前显示语言Languages："<<g_language;
    }else{
        if(g_translator.load(QString(":/i18n/%1").arg(en_US))){
            // ui->pushButton_language_switch->setText("English");
            g_language = en_US;
            qApp->installTranslator(&g_translator);
            qDebug()<<"当前显示语言Languages："<<g_language;
        }else{
            // show_message(tr("fail language switch:%1").arg(en_US), LogLevel::Warning);
        }
    }
    // gSettings->setValue("language",g_language);
    LocalValueSet("language",g_language);
    ui->retranslateUi(this);
    // qDebug() << "读取配置中的语言"<<gSettings->value("language").toString();
}

void MainWindow::on_pushButton_test_clicked()
{
    static int num =0;
    // 使用方法
    // QMessageBox *msgBox = new QMessageBox(this);
    // msgBox->setText("消息内容");
    // msgBox->show();
    HttpDialog *http = new HttpDialog(this,30);
    http->setWindowModality(Qt::ApplicationModal); // 不设置的话,失去焦点不会显示
    http->setAttribute(Qt::WA_DeleteOnClose);// 不设置的话,不会自动析构
    http->get("http://127.0.0.1:80/get",
    [&](const QByteArray& data){
        QJsonObject obj = QJsonValue::fromJson(data).toObject();
        qDebug() << "data"<<obj;
        ui->pushButton_test->setText(tr("Test:%1").arg(num++));
        },
    [&](const QString& error){
        ui->pushButton_test->setText(tr("Test:%1").arg(num--));
        show_message(error,LogLevel::Error);
    });
}

