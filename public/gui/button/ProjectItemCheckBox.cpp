#include "ProjectItemCheckBox.h"
#include "ui_ProjectItemCheckBox.h"

ProjectItemCheckBox::ProjectItemCheckBox(const  FileInfoDetails& project,QWidget *parent)
    : QWidget(parent),project(project)
    , ui(new Ui::ProjectItemCheckBox)
{
    ui->setupUi(this);

    QJsonObject content = project.data;
    ui->label_project_name->setText(tr("项目名称:%1").arg(content.value(JSON_PROJECT_NAME).toString()));
    ui->label_time_created->setText(tr("创建时间:%1").arg(content.value(JSON_CREATE_TIME).toString()));
    ui->label_project_creator->setText(tr("创建人:%1").arg(content.value(JSON_CREATOR).toString()));
    ui->label_project_route->setText(tr("采集路线:%1").arg(content.value(JSON_LINE_NAME).toString()));
    ui->label_project_remark->setText(tr("备注:%1").arg(content.value(JSON_NOTE).toString()));
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

void ProjectItemCheckBox::onCurrentProjectChanged()
{
    if(gProjectFileInfo.isNull()) {
        gProjectFileInfo = QSharedPointer<FileInfoDetails>(new FileInfoDetails(project));
    } else {
        *gProjectFileInfo = project;// 指针不为空时，直接赋值
    }
    emit gControl.onProjectClicked();
}

