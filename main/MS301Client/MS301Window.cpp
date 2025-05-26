#include "MS301Window.h"

MS301Window::MS301Window(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);//无边框和标题栏

}

// #include"button/ProjectItemRadioButton.h"
void MS301Window::AddProject(QJsonObject project)
{
    // ProjectItemRadioButton* button_project(project);

}

void MS301Window::UpdateProjects()
{

}

void MS301Window::GotoHomePage()
{
    qDebug() <<"#MS301Window::GotoHomePage()";
    ui.MainStackedWidget->setCurrentWidget(ui.PageHome);
}

void MS301Window::on_actionReturnPageHome_triggered()
{
    GotoHomePage();
}


void MS301Window::on_pushButton_project_hub_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageProjectHub);
}


void MS301Window::on_pushButton_system_settings_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageSystem);
}


void MS301Window::on_pushButton_device_management_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageDeviceManagement);
}


void MS301Window::on_pushButton_parameter_templates_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageTemplates);
}


void MS301Window::on_pushButton_data_copy_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageDataHub);
}


void MS301Window::on_pushButton_project_start_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageProjectHub);
}


void MS301Window::on_toolButton_alram_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageLog);
}

#include"dialog/CreateNewProject.h"
void MS301Window::on_toolButton_add_project_clicked()
{
    CreateNewProject ProjectDialog;
    if(ProjectDialog.exec() != QDialog::Accepted) return;

    //添加json数据
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    projects[ProjectDialog.project.name] = ProjectDialog.project.ToJsonObject();
    gTaskManager.data[cKeyContent] = projects;
    //执向当前项
    currentProjectItem = ProjectDialog.project;
}

