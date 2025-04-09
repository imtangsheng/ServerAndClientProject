#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H
#include"global.h"
#include <QtWebSockets/QWebSocket>
//inline QWebSocket client;
#define gController MainController::instance()
class MainController
{
public:
	static MainController& instance() {//使用引用,返回其静态变量,不进行拷贝数据
		static MainController instance;
		return instance;
	}
    void initialize();
    QWebSocket webSocketClient;//QWebSocket 依赖于 Qt 事件循环和其他 Qt 基础设施，这些在 QApplication/QCoreApplication 初始化之前是不可用的
// public slots:
    Result handleSession(Session &session,quint8 sTimeout = 30);
private:
	MainController();
    ~MainController();
};
#define gClient MainController::instance().webSocketClient

#include <QGlobalStatic>

class MySingleton {
public:
    void doSomething() { /*...*/ }
// private:
    MySingleton() {
        qDebug()<<"Q_GLOBAL_STATIC(MySingleton, mySingletonInstance)";
    }
};

Q_GLOBAL_STATIC(MySingleton, mySingletonInstance)
// 使用 Q_GLOBAL_STATIC_WITH_ARGS 提前初始化
// Q_GLOBAL_STATIC_WITH_ARGS(MySingleton, myInstance,())

#endif // MAINCONTROLLER_H
