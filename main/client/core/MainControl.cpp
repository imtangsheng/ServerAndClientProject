#include <QGraphicsBlurEffect>


MainControl::MainControl(QObject *parent) : QObject(parent)
{
    module_ = share::Shared::GetModuleName(share::ModuleName::manager);
    gShare.RegisterHandler(module_, this);
}

void MainControl::onEnableChanged(bool enable)
{
    qDebug() <<module_<< "模块已经加载,指令控制状态"<<enable;
}

void MainControl::sendTextMessage(const QString &message)
{
    for (const auto& client : std::as_const(sockets)) {//标准 C++，避免容器分离
        if (client) {  // 检查 QPointer 是否有效
            client->sendTextMessage(message);
        }
    }
}

Result MainControl::SendAndWaitResult(Session &session,QString info, quint8 sTimeout)
{
    for (const auto& client : std::as_const(sockets)) {//标准 C++，避免容器分离
        if (client == nullptr) {  // 检查 QPointer 是否有效
            continue;
        }
        WaitDialog wait(&session,client,info,sTimeout);
        if(wait.init() || wait.exec() == QDialog::Accepted){
            continue;//QDialog::Accepted
        }else{
            //QDialog::Rejected
            qDebug() <<"#SessionFail:"  <<session.GetRequest();
        }
    }
    return true;
}

void MainControl::SetBackgroudAcrylicEffect(QWidget *dialog)
{
    // 创建模糊效果
    QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(dialog);
    // blurEffect->setBlurRadius(10);
    MainBackgroundWidget->setGraphicsEffect(blurEffect);

}


