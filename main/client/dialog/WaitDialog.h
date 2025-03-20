#ifndef WAITDIALOG_H
#define WAITDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QEventLoop>
#include "global.h"

namespace Ui {
class WaitDialog;
}

class WaitDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitDialog(QWidget *parent = nullptr,Session* session = nullptr,quint8 sTimeout = 30);
    ~WaitDialog();

    Session* session;
    quint8 sMaxTimeout;
    QTimer timer;
    Result init();
    Atomic<bool> hasRuselt{false};
    // void show();
public slots:
    void update_timeout();
    void message_received(const QString& message);

    virtual void showEvent(QShowEvent *) override;;//打开窗口时执行
private:
    Ui::WaitDialog *ui;
    QEventLoop loop;
};

#endif // WAITDIALOG_H
