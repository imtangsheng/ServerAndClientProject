/*!
 * @file CameraController.h
 * @brief 相机控制器 ->cn
 * @date 2025-02-24
 */
#pragma once
#include "iController.h"
class CameraController: public ControllerBase
{
	Q_OBJECT
public:
	explicit CameraController(QObject* parent = nullptr, const QString& module = "camera");
	~CameraController();
	void initialize()  override;
	Q_INVOKABLE void test(const Session& session);
	Q_INVOKABLE Result save(const QString& jsonFilePath);

	//interface IControllerSDK 
	//速度 1.直接C++调用	1x	最快，
	//2.Q_INVOKABLE通过QMetaObject::invokeMethod	约10-20x	比直接调用慢 
	//3.slot通过信号触发	约25-40x	最慢的调用方式
	Q_INVOKABLE Result scan(const Session& session);
	Q_INVOKABLE Result prepare(const Session& session) override;
	Q_INVOKABLE Result getUpdateFrameInfo(const Session& session);
	Q_INVOKABLE void SetCamerasParams(const Session& session);
	Q_INVOKABLE void SaveCamerasParams(const Session& session);
	Q_INVOKABLE void trigger(const Session& session);
public slots:
	Result open(const Session& session);
	Result start(const Session& session) override;
	Result stop(const Session& session) override;
	//Result pause() override;
	//Result shutdown() override;
	Result show(const Session& session);
};
inline QPointer<CameraController> gCameraController = nullptr;