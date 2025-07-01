#include "ProjectItemCheckBox.h"
#include "ui_ProjectItemCheckBox.h"

ProjectItemCheckBox::ProjectItemCheckBox(const  FileInfoDetails& project,QWidget *parent)
    : QWidget(parent),project(project)
    , ui(new Ui::ProjectItemCheckBox)
{
    ui->setupUi(this);

}

ProjectItemCheckBox::~ProjectItemCheckBox()
{
    delete ui;
}

bool ProjectItemCheckBox::isChecked()
{
    return ui->checkBox_project_selected->isChecked();
}

void ProjectItemCheckBox::SetChecked(bool checked)
{
    ui->checkBox_project_selected->setChecked(checked);
}

