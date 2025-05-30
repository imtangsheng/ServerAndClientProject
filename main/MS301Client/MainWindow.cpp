#include "MainWindow.h"

#include"ChildWindow/TrolleyWidget.h"
#include"ChildWindow/CameraWidget.h"

QPointer<TrolleyWidget>gTrolley;
QPointer<CameraWidget>gCamera;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);//无边框和标题栏

    /*界面控件显示 加载模块*/
    gTrolley = new TrolleyWidget(this);
    gCamera = new CameraWidget(this);

    //界面设置初始化
    ui.GridLayoutProjects->setContentsMargins(30,30,30,30);
    // 设置列拉伸
    ui.GridLayoutProjects->setColumnStretch(0,1);
    ui.GridLayoutProjects->setColumnStretch(1,1);
    ui.GridLayoutProjects->setColumnStretch(2,1);
    QJsonObject project;
    AddProject(project);
    AddProject(project);
    AddProject(project);
    AddProject(project);
    AddProject(project);
    AddProject(project);
    AddProject(project);
    AddProject(project);
    AddProject(project);
    AddProject(project);
    AddProject(project);

}

#include"button/ProjectItemRadioButton.h"
void MainWindow::AddProject(QJsonObject project)
{
    static QList<QWidget*> widgets;
    ProjectItemRadioButton* button_project = new ProjectItemRadioButton(project,ui.ProjectsWidgetContents);
    connect(button_project,&ProjectItemRadioButton::clicked,this,&MainWindow::onEnterProjectClicked);
    ui.GridLayoutProjects->addWidget(button_project);
    widgets.append(button_project);

}

void MainWindow::UpdateProjects()
{

}

void MainWindow::GotoHomePage()
{
    qDebug() <<"#Window::GotoHomePage()";
    ui.MainStackedWidget->setCurrentWidget(ui.PageHome);
}

void MainWindow::onEnterProjectClicked(QJsonObject project)
{
    qDebug() <<"#Window::onEnterProjectClicked(QJsonObject project)";
}

void MainWindow::on_actionReturnPageHome_triggered()
{
    GotoHomePage();
}


void MainWindow::on_pushButton_project_hub_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageProjectHub);
}


void MainWindow::on_pushButton_system_settings_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageSystem);
}


void MainWindow::on_pushButton_device_management_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageDeviceManagement);
}


void MainWindow::on_pushButton_parameter_templates_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageTemplates);
}


void MainWindow::on_pushButton_data_copy_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageDataHub);
}


void MainWindow::on_pushButton_project_start_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageProjectHub);
}


void MainWindow::on_toolButton_alram_clicked()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageLog);
}

#include"dialog/CreateNewProject.h"
void MainWindow::on_toolButton_add_project_clicked()
{
    CreateNewProject ProjectDialog;
    if(ProjectDialog.exec() != QDialog::Accepted) return;

    QJsonObject obj = ProjectDialog.project.ToJsonObject();
    //添加json数据
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    projects[ProjectDialog.project.name] = obj;
    gTaskManager.data[cKeyContent] = projects;
    //执向当前项
    currentProjectItem = ProjectDialog.project;
    AddProject(obj);
    onEnterProjectClicked(obj);//直接进入任务设置页
}

