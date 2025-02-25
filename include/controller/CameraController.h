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
	explicit CameraController(QObject* parent = nullptr, const QString& module = "DvpCamera");
	~CameraController();
	void initialize()  override;
public slots:
	//interface IControllerSDK
	Result  prepare() override;
	Result start() override;
	Result stop() override;
	//Result pause() override;
	//Result shutDown() override;
};
inline QPointer<CameraController> gCameraController = nullptr;