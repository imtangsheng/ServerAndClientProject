#include "CameraController.h"
CameraController::CameraController(QObject* parent, const QString& module) :
	ControllerBase(module, parent) {
	initialize();
}

CameraController::~CameraController()
{
	qDebug() << "DvpCameraController::~DvpCameraController()";
}

void CameraController::initialize()
{
	qDebug() << "#DVP2API:void CameraController::initialize()";
}

Result CameraController::prepare()
{
	qDebug() << "DvpCameraController::prepare()";

	return Result();
}

Result CameraController::start()
{
	qDebug() << "DvpCameraController::start()";
	//toRequestString(int id, const QString & module, const QString & method, const QJsonValue & params)
	emit sigSouthSend(Session::toRequestString(10, selfName,"start",1));
	return Result();
}

Result CameraController::stop()
{
	qDebug() << "DvpCameraController::stop()";
	return Result();
}
