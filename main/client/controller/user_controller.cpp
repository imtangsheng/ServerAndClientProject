#include "user_controller.h"



UserController::UserController(QObject* parent, const QString& module) :
    QObject(parent),selfName(module) {
    // 自动注册到管理器
    qDebug() << tr("控制类管理模块自动注册到管理器:") << selfName;
    gSouth.RegisterHandler(selfName, this);
}

UserController::~UserController() {
}

void UserController::initialize()
{

}

void UserController::prepare()
{

}

void UserController::start()
{

}

void UserController::stop()
{

}

Q_INVOKABLE void UserController::login(const Session& session) {
    qDebug() << "login:" << session.method<<session.message;
    if (session) {
        if(session.params.toInt(0) == gSouth.type){
            qDebug() << "login sucess:" << session.method<<session.message;
            emit g_new_message("login sucess:");
        }else{
            qWarning()<<tr("login error,设备类型验证错误,不正确:")<< session.params.toInt() <<gSouth.type;
        }
    } else {
        qWarning() << "login failed:" << session.method<<session.message;
    }
}

void UserController::acquisition_begins()
{
    gClient.sendTextMessage(Session::RequestString(11,selfName,"acquisition_begins","测试参数"));
}

void UserController::acquisition_begins(const Session &session)
{
    qDebug() << session.message;
}
