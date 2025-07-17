#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // 设置Fusion样式统一风格
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QDir appDir(QCoreApplication::applicationDirPath()); appDir.cdUp();
    qDebug() << "当前应用程序的目录：" << appDir.absolutePath();
    gShare.InitConfigSettings(appDir.absolutePath(), "client");//初始化配置文件路径,名称

    gShare.sessiontype_ = static_cast<int>(SessionType::Client);
    MainWindow window;
    window.show();

    return app.exec();
}
