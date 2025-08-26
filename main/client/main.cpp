#include "MainWindow.h"
// 在cpp文件中（如main.cpp）注册引用类型

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // 设置Fusion样式统一风格
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    qRegisterMetaType<Session>("Session");//不能在全局作用域直接调用函数
    qRegisterMetaType<const Session&>("const Session&");

    QDir appDir(QCoreApplication::applicationDirPath()); appDir.cdUp();
    qDebug() << "当前应用程序的目录：" << appDir.absolutePath();
    gShare.awake(appDir.absolutePath(), "client");//初始化配置文件路径,名称

    gShare.sessiontype_ = static_cast<int>(SessionType::Client);
    MainWindow window;

    gShare.RegisterHandler(sModuleUser, &window);
    window.show();

    return app.exec();
}
