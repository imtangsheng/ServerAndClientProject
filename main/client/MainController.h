#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H
#include"global.h"
#include <QtWebSockets/QWebSocket>
//inline QWebSocket client;
#define gController MainController::instance()
class MainController
{
public:
	static MainController& instance() {//ʹ������,�����侲̬����,�����п�������
		static MainController instance;
		return instance;
	}
    void initialize();
    QWebSocket webSocketClient;//QWebSocket ������ Qt �¼�ѭ�������� Qt ������ʩ����Щ�� QApplication/QCoreApplication ��ʼ��֮ǰ�ǲ����õ�
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
// ʹ�� Q_GLOBAL_STATIC_WITH_ARGS ��ǰ��ʼ��
// Q_GLOBAL_STATIC_WITH_ARGS(MySingleton, myInstance,())

#endif // MAINCONTROLLER_H
