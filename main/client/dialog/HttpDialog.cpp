#include "HttpDialog.h"
#include "ui_HttpDialog.h"

HttpDialog::HttpDialog(QWidget *parent,quint8 sTimeout)
    : QDialog(parent)
    , ui(new Ui::HttpDialog),sMaxTimeout(sTimeout)
{
    ui->setupUi(this);
    // setModal(true); // 必须设置为模态 失去焦点还会显示
    // setWindowFlags(windowFlags() | Qt::Dialog);
}

HttpDialog::~HttpDialog()
{
    qDebug()<<"WaitDialog::~WaitDialog()";
    if(timer.isActive()){
        timer.stop();
    }
    delete ui;
}

void HttpDialog::ShowMessage(const QString &msg)
{
    ui->label_message->setText(msg);
}

void HttpDialog::onShowDialog()
{
    QTimer::singleShot(100, [&](){
        if(hasRuselt) return;
        show();
        // exec();
        timer.setInterval(1000);
        connect(&timer,&QTimer::timeout,this,&HttpDialog::update_timeout);
        timer.start();
    });
}

void HttpDialog::update_timeout()
{
    sMaxTimeout --;
    if(hasRuselt){
        accept();
    }
    if(sMaxTimeout<=0){
        timer.stop();
        close();
    }
    ui->progressBar_wait->setValue(sMaxTimeout);
}
