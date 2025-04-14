/*!
 * @file CameraController.h
 * @brief 相机控制器,用于公共集成,每个应用程序单独自己实现函数定义
 * @date 2025-02-24
 */
#pragma once
#include "iController.h"

class CameraController:public QObject,public iController
{
	Q_OBJECT
public:
	explicit CameraController(QObject* parent = nullptr,const QString& module = "camera");
	~CameraController();
	/*iController API 设备控制的接口方法*/
	void initialize() final;
	//interface IControllerSDK wrapper method implementations
	void  prepare() final;

	Q_INVOKABLE void test(const Session& session);
	//interface IControllerSDK 
	//速度 1.直接C++调用	1x	最快，
	//2.Q_INVOKABLE通过QMetaObject::invokeMethod	约10-20x	比直接调用慢 
	//3.slot通过信号触发	约25-40x	最慢的调用方式
	Q_INVOKABLE void scan(const Session& session);
	Q_INVOKABLE void open(const Session& session);
	Q_INVOKABLE void trigger(const Session& session);


	Q_INVOKABLE void GetUpdateFrameInfo(const Session& session);

	Q_INVOKABLE void SetCamerasParams(const Session& session);
	Q_INVOKABLE void SaveCamerasParams(const Session& session);
	
public slots:
	void onParamsChanged();
	void start() final;
	void stop() final;
	void show(const Session& session);
};
inline QPointer<CameraController> gCameraController = nullptr;//每个应用程序单独实现声明与定义
