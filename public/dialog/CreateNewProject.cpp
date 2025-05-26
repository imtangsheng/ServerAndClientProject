#include "CreateNewProject.h"
#include "ui_CreateNewProject.h"

CreateNewProject::CreateNewProject(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateNewProject)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    // 然后单独设置 placeholder 颜色
    QPalette palette = ui->lineEdit_project_name->palette();
    palette.setColor(QPalette::Normal,QPalette::PlaceholderText, Qt::red);
    ui->lineEdit_project_name->setPalette(palette);
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

    project.data[FRIEND_DEVICE_TYPE] = "201";
    project.data[FRIEND_AUTHOR] = ui->lineEdit_creator->text();

    Session session({ {"id", Session::NextId()}, {"module", sModuleUser}, {"method", "AppNewProject"}, {"params", project.ToJsonObject()}});
    session.socket = &gClient;
    WaitDialog wait(this, &session, 30);// = new WaitDialog(this);
    //100ms 以内防止弹窗显示
    if (wait.init() || wait.exec() == QDialog::Accepted) {
        accept();
    } else {
        qWarning() << session.result;
        ToolTip::ShowText(tr("Create New Project"),session.result.toString());
        return;
    }
}

