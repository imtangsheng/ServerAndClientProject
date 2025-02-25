/*!
 * @file DvpCameraControllerServer.cpp
 * @brief 度申科技线阵相机控制器的实现文件->cn
 * @date 2025-02-24
 */
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
}

Result CameraController::prepare()
{
	qDebug() << "DvpCameraController::prepare()";
	return Result();
}

Result CameraController::start()
{
	qDebug() << "DvpCameraController::start()";
	return Result();
}

Result CameraController::stop()
{
	qDebug() << "DvpCameraController::stop()";
	return Result();
}
