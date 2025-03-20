#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName("WebSocketServer");
	QCoreApplication::setApplicationVersion("1.0");
	// 安装消息处理钩子，重定向QDebug输出
#ifdef QT_NO_DEBUG
	gLog.init("../logs/", "log", LogLevel::Info, false);
	gLog.InstallMessageHandler();
#else
	gLog.init("../logs/", "log", LogLevel::Debug, true);
#endif // QT_DEBUG
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
	qDebug() << "Sortware version:" << __DATE__ << " " << __TIME__ << "Qt version : " << QT_VERSION_STR << "C++ version : " << __cplusplus;
    gSouth.InitConfigSettings(appDir.absolutePath(), "server");//初始化配置文件路径,名称
    gSouth.type = int(SessionType::Server);
	WebSocketServer server(port);
	qDebug() << "server start port:" << port << " source by:" << typeid(server).name();
	server.initialize();
	QObject::connect(&server, &WebSocketServer::closed, &app, &QCoreApplication::quit);
	return app.exec();
}