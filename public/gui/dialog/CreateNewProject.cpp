#include "CreateNewProject.h"
#include "ui_CreateNewProject.h"

CreateNewProject::CreateNewProject(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateNewProject)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    // setWindowModality(Qt::ApplicationModal); // 应用程序级别的模态
    // 添加背景 毛玻璃效果
    gControl.SetBackgroudAcrylicEffect(this);
}

CreateNewProject::~CreateNewProject()
{
    delete ui;
}


void CreateNewProject::on_pushButton_Accepted_clicked() {
    QString name = ui->lineEdit_project_name->text();
    if (name.isEmpty()) {
        ui->lineEdit_project_name->setPlaceholderText(tr("输入不能为空"));
        ui->lineEdit_project_name->setStyleSheet("QLineEdit { placeholder-text-color: red }");
        return;
    }

    //发送给服务器是否可以创建
    project.name = GetProjectName(name);
    project.path = GetProjectPath(name);

    QJsonObject &content = project.data;
    content[JSON_PROJECT_NAME] = name;
    content[JSON_PROJECT_VERSION] = share::Shared::GetVersion();
    content[JSON_CREATOR] = ui->lineEdit_creator->text();
    content[JSON_LINE_NAME] = ui->lineEdit_project_route->text();
    content[JSON_CREATE_TIME] = GetCurrentDateTime();
    content[JSON_NOTE] = ui->textEdit_remark->toPlainText();

    Session session(sModuleUser, "AddNewProject",project.ToJsonObject());
    if(gControl.SendAndWaitResult(session,tr("创建新项目"),tr("正在创建新项目目录信息"),-1)) {
        accept();
    } else {
        qWarning() << session.result;
        ToolTip::ShowText(session.message,-1);
        return;
        // reject();
    }
}

