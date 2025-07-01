#include "ToolTip.h"
#include "ui_ToolTip.h"
#include <QTimer>

ToolTip::ToolTip(QWidget *parent): ToolTip(Ok, "", "", -1, parent)
{
}

// ToolTip::ToolTip(const QString &title, const QString &message, int msecShowTime, QWidget *parent)
//     : ToolTip(parent)
// {
//     ui->label_title_text->setText(title);
//     ui->label_message->setText(message);
//     if(msecShowTime > 0){
//         QTimer::singleShot(msecShowTime, this, &ToolTip::close);
//     }
// }

ToolTip::ToolTip(TipType type, const QString &title, const QString &message, int msecShowTime, QWidget *parent)
    : QDialog(parent),ui(new Ui::ToolTip),_type(type)
{
    ui->setupUi(this);
    // 在你的Dialog构造函数中添加:
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog); // 设置无边框
    // setAttribute(Qt::WA_TranslucentBackground); // 设置背景透明
    setAttribute(Qt::WA_DeleteOnClose);
    //确认和取消按钮的显示
    switch (_type) {
    case Ok:
        ui->pushButton_reject->hide();
        break;
    default:
        break;
    }

    ui->label_title_text->setText(title);
    ui->label_message->setText(message);
    if(msecShowTime > 0){
        QTimer::singleShot(msecShowTime, this, &ToolTip::close);
    }
}

ToolTip::~ToolTip()
{
    delete ui;
}

void ToolTip::ShowText(const QString &text, int msecShowTime)
{
    ShowText(tr("提示"), text, msecShowTime);
}

void ToolTip::ShowText(const QString &title, const QString &message, int msecShowTime)
{
    ToolTip tooltip(title, message, msecShowTime);
    tooltip.exec();
}
