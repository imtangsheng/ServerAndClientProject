#include "HttpDialog.h"
#include "ui_HttpDialog.h"

HttpDialog::HttpDialog(QWidget *parent,quint8 sTimeout)
    : QDialog(parent)
    , ui(new Ui::HttpDialog),sMaxTimeout(sTimeout),timer(this)
{
    ui->setupUi(this);
    // setModal(true); // 必须设置为模态 失去焦点还会显示
    setWindowFlags(windowFlags() | Qt::Dialog);
}

HttpDialog::~HttpDialog()
{
    qDebug()<<"HttpDialog::~HttpDialog()";
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
    }
    if (timer.isActive()) {
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
    QTimer::singleShot(100,this, [this](){ // 添加 this 作为上下文对象 改用 [this] 捕获更明确
        if(hasResult) {
            close(); // 关闭窗口
            // accept();//设置对话框的结果为 QDialog::Accepted（result() == QDialog::Accepted），并触发 finished(int) 信号（result 为 QDialog::Accepted，通常为 1） 调用 accept() 会关闭 QDialog 窗口（hide()）。
            return; // 检查是否有 HTTP 响应
        }
        ui->progressBar_wait->setValue(sMaxTimeout);
        timer.setInterval(1000);
        connect(&timer, &QTimer::timeout, this, &HttpDialog::update_timeout);
        timer.start(); // 启动倒计时
        show(); // 显示对话框
    });
}

void HttpDialog::update_timeout()
{
    ui->progressBar_wait->setValue(sMaxTimeout--);
    if(hasResult){
        close(); // 关闭窗口
    }//不能使用else if ,在有结果下,确保停止计时器
    if(sMaxTimeout<=0){
        // ShowMessage("request timed out");
        timer.stop();
        close();
    }

}
