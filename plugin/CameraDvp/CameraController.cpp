#include "pch.h"

CameraController::CameraController(QObject* parent, const QString& module) :
	ControllerBase(module, parent) {
}

CameraController::~CameraController()
{
	qDebug() << "DvpCameraController::~DvpCameraController()";
}

void CameraController::initialize()
{
	qDebug() << "#DVP2API:void CameraController::initialize()";

}

void CameraController::test(const Session& session)
{
    gCameraSDK->test();
    emit gSigSent(session.ResponseString("CameraController::test", tr("测试成功")));
}

Result CameraController::save(const QString& jsonFilePath)
{
	return gCameraSDK->SaveConfigToFile(jsonFilePath);
}

Result CameraController::scan(const Session& session)
{
	Result result = gCameraSDK->scan();
	if (result) {
		QString deviceNames = gCameraSDK->devicesNamesList.join(", ");
		emit gSigSent(session.ResponseString(deviceNames, tr("扫描相机成功")));
	}
	else {
		emit gSigSent(session.ErrorString(result.code, result.message));
	}
	return result;
}

Result CameraController::prepare(const Session& session)
{
	Result result = gCameraSDK->prepare(session);
	gSouth.on_send(result, session);
	return result;
}

Result CameraController::getUpdateFrameInfo(const Session& session)
{
	Result result = gCameraSDK->slotDispRate();
	gSouth.on_send(result, session);
	return result;
}

void CameraController::SetCamerasParams(const Session& session)
{
    QJsonObject cameraParamsJson = session.params.toObject();
	//#遍历并设置相机参数
	for (const QString& key : cameraParamsJson.keys()) {
		QJsonObject param_camera = cameraParamsJson[key].toObject();
		gCameraSDK->SetCamerasParams(gCameraSDK->cameraInfoArray->handle, param_camera);
	}
	gSouth.on_send(Result::Success("设置相机参数"), session);
}

Q_INVOKABLE void CameraController::SaveCamerasParams(const Session& session) {
	QJsonObject cameraParamsJson = session.params.toObject();
	gCameraSDK->cameraConfigJson["camera_params"] = cameraParamsJson;
	gCameraSDK->SaveConfigToFile();
	gSouth.on_send(Result::Success("保存相机参数"), session);
}

Q_INVOKABLE void CameraController::trigger(const Session& session) {
    Result result = gCameraSDK->triggerFire();
    gSouth.on_send(result, session);
}

Result CameraController::open(const Session& session)
{
	Result result = gCameraSDK->open();
	gSouth.on_send(result, session);
	return result;
}

Result CameraController::start(const Session& session)
{
	Result result = gCameraSDK->start();
	gSouth.on_send(result, session);
	return result;
}

Result CameraController::stop(const Session& session)
{
	Result result = gCameraSDK->stop();
	gSouth.on_send(result, session);
	return result;
}

Result CameraController::show(const Session& session)
{
	Result result = gCameraSDK->Property();
	gSouth.on_send(result, session);
	return result;
}
