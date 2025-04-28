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
    static HttpDialog& instance(QWidget *parent = nullptr){
        static HttpDialog httpclient(parent);
        return httpclient;
    }
    explicit HttpDialog(QWidget *parent = nullptr,quint8 sTimeout = 30);
    ~HttpDialog();

    QNetworkAccessManager manager;
    void get(const QString &url, std::function<void(const QByteArray&)> onSuccess,
             std::function<void(const QString&)> onError)
    {
        QNetworkRequest request(url);
        QNetworkReply *reply = manager.get(request);

        connect(reply, &QNetworkReply::finished, this, [=]() {
            hasRuselt = true;
            if(reply->error() == QNetworkReply::NoError) {
                // 显示成功对话框
                onSuccess(reply->readAll());
                accept();
            } else {
                onError(reply->errorString());
                ShowMessage(reply->errorString());
            }
            reply->deleteLater();

        });
        onShowDialog();
    }

    void ShowMessage(const QString& msg);

    quint8 sMaxTimeout;
    QTimer timer;
    Atomic<bool> hasRuselt{false};

protected slots:
    void onShowDialog();
    void update_timeout();
private:
    Ui::HttpDialog *ui;
};

#endif // HTTPDIALOG_H
