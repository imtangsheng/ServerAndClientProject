#include "WaitDialog.h"
#include "ui_WaitDialog.h"

WaitDialog::WaitDialog(Session* session,QWebSocket* client, quint8 sTimeout,QWidget *parent)
    : QDialog(parent),session(session),pClient(client),sMaxTimeout(sTimeout)
    , ui(new Ui::WaitDialog)
{
    ui->setupUi(this);
    // init();
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog); // 设置无边框
    // setAttribute(Qt::WA_TranslucentBackground); // 设置背景透明
    setAttribute(Qt::WA_DeleteOnClose);
}

WaitDialog::~WaitDialog()
{
    qDebug()<<"WaitDialog::~WaitDialog()";
    if(timer.isActive()){
        timer.stop();
    }
    // gFilter.remove(this->objectName());
    delete ui;
}

Result WaitDialog::init()
{
    // connect(pCilent,&QWebSocket::textMessageReceived,this,&WaitDialog::message_received);
    // gFilter.append(std::bind(&WaitDialog::HandleFilte, this, std::placeholders::_1));
    pClient->sendTextMessage(session->GetRequest());
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")<<" Dialog Send:"<<session->GetRequest();
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

Result WaitDialog::filter(Session &recv)
{
    qDebug() <<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")<< "WaitDialog::HandleFilter:"<<recv.message;
    if(recv.id == session->id && recv.module == session->module && recv.method == session->method){
        session->code = recv.code;
        session->result = recv.result;
        session->message = recv.message;

        loop.quit();

        hasRuselt = true;
        hasRuselt.message = recv.message;
        ui->label_show->setText(hasRuselt.message);
        accept();
        qDebug()<<"会设置dialog的result为QDialog::Accepted（值为1）";
        return Result(true,recv.message);
    }else{
        qWarning() << session->id << session->module <<session->method;
        return false;
    }
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


// void WaitDialog::showEvent(QShowEvent *)
// {

// }
