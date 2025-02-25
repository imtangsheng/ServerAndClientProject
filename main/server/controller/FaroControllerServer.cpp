/*!
 * @file FaroControllerServer.cpp
 * @brief 法如控制器的实现文件->cn
 * @date 2025-02-24
 */
#include "FaroController.h"

FaroController::FaroController(QObject* parent, const QString& module) :
	ControllerBase(module,parent) {
	initialize();
}

FaroController::~FaroController()
{
	qDebug()<< "FaroController::~FaroController()";
	if (timer) {
		timer->stop();
		timer->deleteLater();
	}
}

void FaroController::initialize()
{
	selfThread = new QThread();
	selfThread->setObjectName("FaroControllerThread");
	timer = new QTimer();
	timer->moveToThread(selfThread);
	selfThread->start();
}

Result FaroController::prepare()
{
	qDebug() << "FaroController::prepare()";
	return Result();
}

Result FaroController::start()
{
	qDebug() << "FaroController::start()";
	return Result();
}

Result FaroController::stop()
{
	qDebug() << "FaroController::stop()";
	return Result();
}

Result FaroController::shutDown()
{
	qDebug() << "FaroController::shutDown()";
	return Result();
}
