#include "ParamItemQRadioBox.h"
#include "ui_ParamItemQRadioBox.h"
#include <QButtonGroup>
QButtonGroup *gRadioGroup = new QButtonGroup(nullptr);
ParamItemQRadioBox::ParamItemQRadioBox(int id,QJsonObject param,QWidget *parent)
    : QWidget(parent),id(id)
    , ui(new Ui::ParamItemQRadioBox)
{
    ui->setupUi(this);
    qDebug()<<"ParamItemQRadioBox:" << param;
    gRadioGroup->addButton(ui->radioButton_project_selected);
    ui->label_id->setText(QString("%1").arg(this->id+1, 2, 10, QChar('0')));


}

ParamItemQRadioBox::~ParamItemQRadioBox()
{
    delete ui;
}

void ParamItemQRadioBox::SetChecked(bool checked)
{
    ui->radioButton_project_selected->setChecked(true);
}

