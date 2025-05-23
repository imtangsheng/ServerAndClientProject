#include "MS301Window.h"

#include "ProjectHub.h" //项目库

MS301Window::MS301Window(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);//无边框和标题栏

    //添加界面元素
    ui.MainStackedWidget->addWidget(new ProjectHub(this));
}

void MS301Window::GotoHomePage()
{
    ui.MainStackedWidget->setCurrentWidget(ui.PageHome);
}

void MS301Window::on_toolButton_logo_clicked()
{
    GotoHomePage();
}


void MS301Window::on_pushButton_project_hub_clicked()
{
    // 通过objectName查找widget
    QWidget* widget = ui.MainStackedWidget->findChild<QWidget*>("ProjectHub");
    if(widget) {
        ui.MainStackedWidget->setCurrentWidget(widget);
    }
}

