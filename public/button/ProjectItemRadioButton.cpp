#include "ProjectItemRadioButton.h"
#include "ui_ProjectItemRadioButton.h"

ProjectItemRadioButton::ProjectItemRadioButton(const  QJsonObject& project,QWidget *parent)
    : QWidget(parent),m_project(project)
    , ui(new Ui::ProjectItemRadioButton)
{
    ui->setupUi(this);

}

ProjectItemRadioButton::~ProjectItemRadioButton()
{
    delete ui;
}

void ProjectItemRadioButton::SetChecked(bool checked)
{

}
