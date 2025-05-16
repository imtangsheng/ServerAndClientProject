#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H
#include"global.h"
#include <QtWebSockets/QWebSocket>
//inline QWebSocket client;
#define gController MainController::instance()

//不可拷贝复制,也不可以先于qApp先初始化
#define gClient GetWebSocketClient()
inline QSharedPointer<QWebSocket> gSocket{nullptr};
inline QWebSocket& GetWebSocketClient(){
    static QWebSocket socketClient;
    return socketClient;
}

class MainController
{
public:
	static MainController& instance() {//使用引用,返回其静态变量,不进行拷贝数据
		static MainController instance;
		return instance;
	}
    void initialize();
// public slots:
    Result handleSession(Session &session,quint8 sTimeout = 30);
private:
	MainController();
    ~MainController();
};


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
