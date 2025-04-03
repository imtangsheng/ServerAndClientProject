#include "CameraController.h"
CameraController::CameraController(QObject* parent)
    :QObject(parent)
{
	gSouth.RegisterHandler(sModuleCamera, this);
}

CameraController::~CameraController()
{
	qDebug() << "DvpCameraController::~DvpCameraController()";
}

void CameraController::initialize()
{
	qDebug() << "#DVP2API:void CameraController::initialize()";

}

void CameraController::prepare()
{

}

void CameraController::onParamsChanged() {
	emit gSigSent(Session::RequestString(2, sModuleCamera, "", QJsonArray{ gCameraSDK->cameraConfigJson }));
}

void CameraController::start()
{
	gCameraSDK->start();
}

void CameraController::stop()
{
	gCameraSDK->stop();
}

void CameraController::test(const Session& session)
{
    gCameraSDK->test();
    emit gSigSent(session.ResponseString(tr("测试成功")));
}


void CameraController::scan(const Session& session)
{
	Result result = gCameraSDK->scan();
	if (result) {
		QString deviceNames = gCameraSDK->devicesNamesList.join(", ");
		Session session1 = session;
        session1.result = deviceNames;
		emit gSigSent(session1.ResponseString(tr("扫描相机成功")));
	}
	else {
		emit gSigSent(session.ErrorString(result.code, result.message));
	}

}

void CameraController::open(const Session& session) {
	Result result = gCameraSDK->open();
	gSouth.on_send(result, session);
}

void CameraController::GetUpdateFrameInfo(const Session& session)
{
	Result result = gCameraSDK->slotDispRate();
	gSouth.on_send(result, session);
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


void CameraController::show(const Session& session)
{
	Result result = gCameraSDK->Property();
	gSouth.on_send(result, session);
}


