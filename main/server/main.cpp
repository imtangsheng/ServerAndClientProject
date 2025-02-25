#include <QCoreApplication>
#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include "WebSocketServer.h"
int main(int argc, char* argv[])
{
	QCoreApplication app(argc, argv);
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
	qDebug() << "Qt version: " << QT_VERSION_STR << "C++ version:" << __cplusplus;
	WebSocketServer server(port);
	qDebug() << "server start port:" << port << " source by:" << typeid(server).name();

	QObject::connect(&server, &WebSocketServer::closed, &app, &QCoreApplication::quit);
	return app.exec();
}