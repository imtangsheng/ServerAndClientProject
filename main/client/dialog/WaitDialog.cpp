#include "WaitDialog.h"
#include "ui_WaitDialog.h"

WaitDialog::WaitDialog(QWidget *parent,Session* session, quint8 sTimeout)
    : QDialog(parent),session(session),sMaxTimeout(sTimeout)
    , ui(new Ui::WaitDialog)
{
    ui->setupUi(this);
    // init();
}

WaitDialog::~WaitDialog()
{
    qDebug()<<"WaitDialog::~WaitDialog()";
    if(timer.isActive()){
        timer.stop();
    }
    delete ui;
}

Result WaitDialog::init()
{
    QWebSocket* pCilent = qobject_cast<QWebSocket*>(this->session->socket);
    if(!pCilent){
        qWarning()<<"网络未连接";
        pCilent = &gClient;
    }
    connect(pCilent,&QWebSocket::textMessageReceived,this,&WaitDialog::message_received);
    pCilent->sendTextMessage(session->getRequest());
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")<<" Dialog Send:"<<session->getRequest();
    QTimer::singleShot(100, &loop, &QEventLoop::quit);
    loop.exec();
    if(hasRuselt) return hasRuselt;

    if(!this->session->message.isEmpty()){
        ui->label_show->setText(this->session->message);
    }
    ui->progressBar_wait->setMaximum(sMaxTimeout);
    ui->progressBar_wait->setValue(sMaxTimeout);

    timer.setInterval(1000);
    connect(&timer,&QTimer::timeout,this,&WaitDialog::update_timeout);
    timer.start();
    return false;
}

void WaitDialog::update_timeout()
{
    sMaxTimeout --;
    if(hasRuselt){
        // qDebug() << "update_timeout()"<<hasRuselt.message;
        // ui->label_show->setText(hasRuselt.message);
        // QThread::sleep(3);
        accept();
    }
    if(sMaxTimeout<=0){
        timer.stop();
        // reject();//关闭对话框并返回 QDialog::Rejected
        // 使用done() 退出
        // done(QDialog::Rejected);
        // 使用close() 退出
        close();
        // 使用事件队列确保对话框关闭 可以退出
        // QMetaObject::invokeMethod(this, "reject", Qt::QueuedConnection);

    }
    //%p%: 百分比, 这是默认的显示方式
    // %v: 当前进度
    ui->progressBar_wait->setValue(sMaxTimeout);
}

void WaitDialog::message_received(const QString &message)
{
    qDebug() <<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")<< "WaitDialog::message_received(const QString:"<<message;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {//向客户端发送错误信息
        // reject();//关闭对话框并返回 QDialog::Rejected
        qDebug()<<"jsonDoc.isNull:WaitDialog::message_received(const QString &"<<message;
        return;
    }

    Session recv(jsonDoc.object());
    if(recv.id == session->id && recv.module == session->module && recv.method == session->method){
        session->code = recv.code;
        session->result = recv.result;
        session->message = recv.message;

        loop.quit();

        hasRuselt = true;
        hasRuselt.message = recv.message;
        ui->label_show->setText(hasRuselt.message);
        accept();
    }else{
        qWarning() << ":message_received(const QString)"<<message;
        qWarning() << session->id << session->module <<session->method;
    }
}

void WaitDialog::showEvent(QShowEvent *)
{

}
