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
    int id;
    // QString text;

    // bool isChecked();
    void SetChecked(bool checked);

private:
    Ui::ParamItemQRadioBox *ui;

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        SetChecked(true);
        emit gControl.onParamTemplateClicked(id);
    }
};

#endif
