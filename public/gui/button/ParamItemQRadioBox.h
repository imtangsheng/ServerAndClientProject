#ifndef PARAMITEMQRADIOBOX_H
#define PARAMITEMQRADIOBOX_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class ParamItemQRadioBox;
}

class ParamItemQRadioBox : public QWidget
{
    Q_OBJECT
public:
    explicit ParamItemQRadioBox(int id,QJsonObject param,QWidget *parent = nullptr);
    ~ParamItemQRadioBox();
    int index;//序列号,json数组
    void ShowParam(QJsonObject param);
    bool isChecked();
    void SetChecked(bool checked);

private:
    Ui::ParamItemQRadioBox *ui;

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        SetChecked(true);
        emit gControl.onParamTemplateClicked(index);
    }
};

#endif
