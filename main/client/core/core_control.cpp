#include <QGraphicsBlurEffect>

void CoreControl::sendTextMessage(const QString &message)
{
    for (const auto& client : std::as_const(sockets)) {//标准 C++，避免容器分离
        if (client) {  // 检查 QPointer 是否有效
            client->sendTextMessage(message);
        }
    }
}

Result CoreControl::SendAndWaitResult(Session &session, quint8 sTimeout)
{
    for (const auto& client : std::as_const(sockets)) {//标准 C++，避免容器分离
        if (client == nullptr) {  // 检查 QPointer 是否有效
            continue;
        }
        WaitDialog wait(&session,client,sTimeout);
        if(wait.init() || wait.exec() == QDialog::Accepted){
            continue;//QDialog::Accepted
        }else{
            //QDialog::Rejected
            qDebug() <<"#SessionFail:"  <<session.GetRequest();
        }
    }
    return true;
}

void CoreControl::SetBackgroudAcrylicEffect(QWidget *dialog)
{
    // 创建模糊效果
    QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(dialog);
    // blurEffect->setBlurRadius(10);
    MainBackgroundWidget->setGraphicsEffect(blurEffect);

}
