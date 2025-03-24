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

public slots:
    Result handleSession(Session &session,quint8 sTimeout = 30);
private:
	MainController();
};
#define gClient MainController::instance().webSocketClient
#endif // MAINCONTROLLER_H
