#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class ToolTip;
}

class ToolTip : public QDialog
{
    Q_OBJECT
public:
    enum TipType:quint8 {
        Confirm,
        Ok
    };
    // 基础构造函数
    explicit ToolTip(QWidget *parent = nullptr);
    // 委托构造函数
    explicit ToolTip(const QString& title, const QString& message, int msecShowTime = -1, QWidget *parent = nullptr)
        : ToolTip(Ok, title, message, msecShowTime, parent) {}
    // 主要构造函数
    explicit ToolTip(TipType type,const QString& title,const QString& message,int msecShowTime = -1,QWidget *parent = nullptr);
    ~ToolTip();

    static void ShowText(const QString &text, int msecShowTime = 3000);
    static void ShowText(const QString& title,const QString& message,int msecShowTime = 3000);

private:
    Ui::ToolTip *ui;
    TipType _type;
    QTimer timer;
};

#endif // TOOLTIP_H
