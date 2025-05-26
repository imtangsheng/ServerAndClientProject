#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QDialog>

namespace Ui {
class ToolTip;
}

class ToolTip : public QDialog
{
    Q_OBJECT

public:
    explicit ToolTip(QWidget *parent = nullptr);
    explicit ToolTip(const QString& title,const QString& message,int msecShowTime = -1,QWidget *parent = nullptr);
    ~ToolTip();

    static void ShowText(const QString &text, int msecShowTime = 3000);
    static void ShowText(const QString& title,const QString& message,int msecShowTime = 3000);

private:
    Ui::ToolTip *ui;
};

#endif // TOOLTIP_H
