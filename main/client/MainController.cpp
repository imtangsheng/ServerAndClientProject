#include "MainController.h"

void MainController::initialize()
{
    qDebug()<<"MainController::initialize()";
}


Result MainController::handleSession(Session &session,quint8 sTimeout)
{
    session.socket = &webSocketClient;
    WaitDialog wait(nullptr,&session,sTimeout);
    //100ms 以内防止弹窗显示
    if(wait.init() || wait.exec() == QDialog::Accepted){
        qDebug() << " QDialog::Accepted";
        return true;
    }else{
        qDebug() << "QDialog::Rejected"<<QDialog::Rejected;
        return false;
    }
}

MainController::MainController() {
    qDebug()<<"MainController::MainController()";
}

MainController::~MainController()
{
    qDebug()<<"MainController::~MainController()";
}
