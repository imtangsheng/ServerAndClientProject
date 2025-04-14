/*!
 * @file CameraPlugin.cpp
 * @brief 度申相机DVP2(Digital Videl Platform 2) SDK 的API开发的实现文件->cn
 * @date 2025-02
 */
#include "CameraPlugin.h"
CameraPlugin::CameraPlugin()
{
	IPluginDevice::initialize();
}

CameraPlugin::~CameraPlugin()
{
	qDebug() << "#PluginCamera~CameraPlugin()";
}

QString CameraPlugin::_module() const {
	return south::ShareLib::GetModuleName(south::ModuleName::camera);
}

void CameraPlugin::initialize()
{
	//qDebug() <<"#测试:"<<&gController<<QThread::currentThreadId();
	gCameraController = new CameraController(this,_module());
	//south::ShareLib::instance().registerHandler("camera", gCameraController.data());
	//bool enable =	south::ShareLib::GetConfigSettings()->value("camera/enable",false).toBool();
	//if (!enable) {
		//south::ShareLib::GetConfigSettings()->setValue("camera/enable", true);
	//}
	gCameraSDK = new CameraSDK(this);
}


Result CameraPlugin::disconnect()
{
	qDebug() << "#PluginCamera断开函数";
	return Result();
}

Result CameraPlugin::AcquisitionStart()
{
	qDebug() << "#PluginCamera开始采集函数";
	return Result(true);
}

Result CameraPlugin::AcquisitionStop()
{
	qDebug() << "#PluginCamera停止采集函数";
	return Result(true);
}

Result CameraPlugin::SetParameters(const QJsonObject& parameters)
{
	qDebug() << "#PluginCamera设置参数函数"<<parameters;
	return Result();
}

QString CameraPlugin::name() const
{
	return QString("camera");
}

QString CameraPlugin::version() const
{

	return QString("0.0.1");
}

Result CameraPlugin::execute(const QString& method)
{
	qDebug() << "#PluginCamera执行函数"<<method;
	return Result();
}


