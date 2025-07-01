#include "CreateNewProject.h"
#include "ui_CreateNewProject.h"
// #include <QGraphicsDropShadowEffect>
CreateNewProject::CreateNewProject(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateNewProject)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    // 添加阴影效果
    // QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
    // shadow->setBlurRadius(40);
    // shadow->setColor(QColor(0, 0, 0, 64));
    // this->setGraphicsEffect(shadow);

}

CreateNewProject::~CreateNewProject()
{
    delete ui;
}


void CreateNewProject::on_pushButton_Accepted_clicked() {
    QString name = ui->lineEdit_project_name->text();
    if (name.isEmpty()) {
        ui->lineEdit_project_name->setPlaceholderText(tr("This cannot be empty"));
        ui->lineEdit_project_name->setStyleSheet("QLineEdit { placeholder-text-color: red }");
        return;
    }

    //发送给服务器是否可以创建
    project.name = GetProjectName(ui->lineEdit_project_name->text());
    project.path = GetProjectPath(project.name);


    project.data[FRIEND_PROJECT_NAME] = name;
    project.data[FRIEND_PROJECT_VERSION] = share::Shared::GetVersion();

    QJsonObject content;
    content[FRIEND_DEVICE_TYPE] = "201";
    content[FRIEND_AUTHOR] = ui->lineEdit_creator->text();
    content[FRIEND_LINE_NAME] = ui->lineEdit_project_route->text();

    content[FRIEND_CREATE_TIME] = QDateTime::currentDateTime().toString(kTimeFormat);
    content[FRIEND_NOTE] = ui->textEdit_remark->toPlainText();

    project.data[FRIEND_PROJECT_CONTENT] = content;

    Session session({ {"id", Session::NextId()}, {"module", sModuleUser}, {"method", "AppNewProject"}, {"params", project.ToJsonObject()}});
    if(gControl.SendAndWaitResult(session)) {
        accept();
    } else {
        qWarning() << session.result;
        ToolTip::ShowText(tr("Create New Project"),session.result.toString());
        return;
    }
}

