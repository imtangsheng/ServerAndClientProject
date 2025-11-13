#include <QCommandLineParser>
#include <QCommandLineOption>
#include "UserServer.h"

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("WebSocketServer");
    QCoreApplication::setApplicationVersion("1.0");
    QCommandLineParser parser;
    parser.setApplicationDescription("QtWebSockets server");
    parser.addHelpOption();
    QCommandLineOption portOption(QStringList() << "p" << "port",
        QCoreApplication::translate("main", "Port for server [default: 8080]."),
        QCoreApplication::translate("main", "port"), QLatin1String("8080"));
    parser.addOption(portOption);
    parser.process(app);

    int port = parser.value(portOption).toInt();
    // 打印Qt的C++版本
    QDir appDir(QCoreApplication::applicationDirPath()); appDir.cdUp();
    qDebug() << "当前应用程序的目录：" << appDir.absolutePath();
    qDebug() << "软件运行时间: " << __DATE__ << " " << __TIME__ << "Qt 版本:" << QT_VERSION_STR << "C++ 版本:" << __cplusplus;
    gShare.session_type_ = int(SessionType::Server);
    gShare.awake(appDir.absolutePath(), "server");//初始化配置文件路径,名称, 变量gSettings 等
    // 安装消息处理钩子，重定向QDebug输出
#ifdef QT_NO_DEBUG
    LogLevel logLevel = static_cast<LogLevel>(gSettings->value("LogLevel", static_cast<int>(LogLevel::Info)).toInt());
    qDebug() << "当前日志等级:" << int(logLevel);
    gLog.init("../logs/", "log", logLevel, false);
    gLog.InstallMessageHandler();
#else
    gLog.init("../logs/", "log", LogLevel::Debug, true);
#endif // QT_NO_DEBUG
    // 设置语言
    QString language = gSettings->value("language").toString();
    if (language.isEmpty()) {
        QString locale = QLocale::system().name();
        language = QLocale(locale).name();
    }
    gShare.language = language;
    QObject::connect(&gShare, &share::Shared::signal_translator_load, &app, [&app](QTranslator& translator,bool isLoad) {
        if (isLoad) {
            app.installTranslator(&translator);
        } else {
            app.removeTranslator(&translator);
        }
    });
    UserServer user(&app);
    user.initialize(port,12345);//因为80端口给 http代理,故使用其他 http的端口
    qDebug() << "WebSocket server listening on port" << port;
    gShare.RegisterHandler(user.module_, &user);

    return app.exec();
}