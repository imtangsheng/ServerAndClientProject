#include "ParamItemQRadioBox.h"
#include "ui_ParamItemQRadioBox.h"
#include <QButtonGroup>
QButtonGroup *gRadioGroup = new QButtonGroup(nullptr);

ParamItemQRadioBox::ParamItemQRadioBox(int id,QJsonObject param,QWidget *parent)
    : QWidget(parent),index(id)
    , ui(new Ui::ParamItemQRadioBox)
{
    ui->setupUi(this);
    qDebug()<<"ParamItemQRadioBox:" << param;
    // gRadioGroup->setExclusive(false);// 设置为非独占模式 允许多选或全不选
    gRadioGroup->addButton(ui->radioButton_project_selected);
    ui->label_id->setText(QString("%1").arg(this->index+1, 2, 10, QChar('0')));
    ShowParam(param);
}

ParamItemQRadioBox::~ParamItemQRadioBox()
{
    delete ui;
}

void ParamItemQRadioBox::ShowParam(QJsonObject param)
{
    ui->label_name->setText(param[JSON_TEMPLATE].toString());
    ui->label_tunnel_type->setText(param[Json_TunnelType].toString());
    ui->doubleSpinBox_template_tunnel_diameter->setValue(param[JSON_DIAMETER].toDouble());
    ui->spinBox_template_car_speed->setValue(param[JSON_SPEED].toInt());
    ui->doubleSpinBox_accuracy->setValue(param[JSON_ACCURACY].toDouble());//int类型
    ui->label_camera_param_template->setText(param[Json_CameraTemplate].toString());
}

bool ParamItemQRadioBox::isChecked()
{
    return ui->radioButton_project_selected->isChecked();
}

void ParamItemQRadioBox::SetChecked(bool checked)
{
    ui->radioButton_project_selected->setChecked(true);
}

