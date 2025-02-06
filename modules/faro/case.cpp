// Test.cpp
#include <QCoreApplication>
#include "RealtimeSolving.h"
#include "logger.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    Logger::getInstance()->init("logs", "myapp");
    // 安装消息处理钩子，重定向QDebug输出
#ifdef QT_NO_DEBUG
    Logger::getInstance()->installMessageHandler();
#endif // QT_DEBUG
    static QDateTime startTime = QDateTime::currentDateTime();
    RealtimeSolving solviong;
    solviong.test();
    qDebug() << "#test end" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
    //return a.exec();
}
