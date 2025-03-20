/*!
 * @file CameraPlugin.cpp
 * @brief 度申相机DVP2(Digital Videl Platform 2) SDK 的API开发的实现文件->cn
 * @date 2025-02
 */
#include "CameraPlugin.h"
CameraPlugin::CameraPlugin()
{
	qDebug() << "#PluginCamera构造函数";
	//initialize();
}

CameraPlugin::~CameraPlugin()
{
	qDebug() << "#PluginCamera析构函数";
}

Result CameraPlugin::initialize()
{
	//qDebug() <<"#测试:"<<&gController<<QThread::currentThreadId();
	gCameraController = new CameraController(this);
	//south::ShareLib::instance().registerHandler("camera", gCameraController.data());
	bool enable =	south::ShareLib::GetConfigSettings()->value("camera/enable",false).toBool();
	if (!enable) {
		south::ShareLib::GetConfigSettings()->setValue("camera/enable", true);
	}
	gCameraSDK = new CameraSDK(this);
	qDebug() << "#PluginCamera初始化函数"<< enable;
	return Result();
}

Result CameraPlugin::connect()
{
	qDebug() << "#PluginCamera连接函数";
	return Result();
}

Result CameraPlugin::disconnect()
{
	qDebug() << "#PluginCamera断开函数";
	return Result();
}

Result CameraPlugin::AcquisitionStart()
{
	qDebug() << "#PluginCamera开始采集函数";
	return Result();
}

Result CameraPlugin::AcquisitionStop()
{
	qDebug() << "#PluginCamera停止采集函数";
	return Result();
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
