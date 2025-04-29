/*
方法	关闭窗口	销毁（Qt::WA_DeleteOnClose）	信号触发	结果设置
accept()	是	是	accepted(), finished(QDialog::Accepted)	QDialog::Accepted (1)
reject()	是	是	rejected(), finished(QDialog::Rejected)	QDialog::Rejected (0)
close()	是	是	finished(result)	不改变（保持上次设置）
hide()	是（隐藏）	是（如果 Qt::WA_DeleteOnClose 启用）	无（除非销毁触发 finished(result)）	不改变
done(int result)	是	是	finished(result)	自定义（例如 0 或其他值）
*/
#ifndef HTTPDIALOG_H
#define HTTPDIALOG_H

#include <QDialog>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

namespace Ui {
class HttpDialog;
}

class HttpDialog : public QDialog
{
    Q_OBJECT
public:
    explicit HttpDialog(QWidget *parent = nullptr,quint8 sTimeout = 30);
    ~HttpDialog();

    //使用局部静态,重复构造释放
    static QNetworkAccessManager& http(){
        static QNetworkAccessManager manager;
        return manager;
    };
/*
 * Lambda 捕获问题 [&] 捕获了临时变量（如 onSuccess/onError），当回调触发时这些对象可能已被销毁
 * 回调函数生命周期：std::function 对象在 get() 调用结束后被销毁 但网络请求是异步的，回调触发时原始函数对象已不存在
 * 尝试调用已释放的 std::function 导致 WASM 内存访问越界
解决方案
方案1：使用值捕获[=] + 智能指针（推荐）:[&]每次onError地址不变,使用[=]会变 ,可是是引用捕获了该局部变量
方案2：使用成员变量存储回调
*/
    void get(const QString &url, std::function<void(const QByteArray&)> onSuccess,
             std::function<void(const QString&)> onError)
    {
        QNetworkRequest request(url);
        currentReply = http().get(request);
        hasResult = false;
        // 使用智能指针管理回调生命周期
        // auto successPtr = std::make_shared<std::function<void(QByteArray)>>(onSuccess);
        // auto errorPtr = std::make_shared<std::function<void(QString)>>(onError);
        connect(currentReply, &QNetworkReply::finished, this, [=]() {
            hasResult = true;
            if(currentReply->error() == QNetworkReply::NoError) {
                onSuccess(currentReply->readAll());
            } else {
                qDebug() <<"currentReply is error:"<<currentReply->error()<<currentReply->errorString();
                // 直接使用errorString()已经包含可读信息
                onError(tr("request failed: %1 (code %2)")
                            .arg(currentReply->errorString())
                            .arg(currentReply->error()));
                // ShowMessage(error);
            }
            currentReply->deleteLater();
            currentReply = nullptr;
        });
        onShowDialog();//需要过一定时间判断是否返回,确保生命周期
    }

    void ShowMessage(const QString& msg);
    QNetworkReply *currentReply{nullptr};
    quint8 sMaxTimeout;
    QTimer timer;
    bool hasResult{false};//（WASM 是单线程的，无需 std::atomic）
    void onShowDialog();
protected slots:
    void update_timeout();
private:
    Ui::HttpDialog *ui;
};

#endif // HTTPDIALOG_H
