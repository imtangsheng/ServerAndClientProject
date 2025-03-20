/*!
 * @file TrolleyPlugin.cpp
 * @brief 度申相机DVP2(Digital Videl Platform 2) SDK 的API开发的实现文件->cn
 * @date 2025-02
 */
#include "pch.h"
#include "TrolleyPlugin.h"
TrolleyPlugin::TrolleyPlugin()
{
	qDebug() << "Trolley构造函数";
	//initialize();
}

TrolleyPlugin::~TrolleyPlugin()
{
	qDebug() << "Trolley析构函数";
}

Result TrolleyPlugin::initialize()
{
	qDebug() << "Trolley初始化函数";
	
	//qDebug() <<"#测试:"<<&gController<<QThread::currentThreadId();
	gTrolleyController = new TrolleyController(this);
	//south::Share::instance().registerHandler("camera", gCameraController.data());
	return Result();
}

Result TrolleyPlugin::connect()
{
	qDebug() << "Trolley连接函数";
	return Result();
}

Result TrolleyPlugin::disconnect()
{
	qDebug() << "Trolley断开函数";
	return Result();
}

Result TrolleyPlugin::AcquisitionStart()
{
	qDebug() << "Trolley开始采集函数";
	return Result();
}

Result TrolleyPlugin::AcquisitionStop()
{
	qDebug() << "Trolley停止采集函数";
	return Result();
}

Result TrolleyPlugin::SetParameters(const QJsonObject& parameters)
{
	qDebug() << "Trolley设置参数函数"<<parameters;
	return Result();
}

QString TrolleyPlugin::name() const
{
	return QString("TrolleyPlugin");
}

QString TrolleyPlugin::version() const
{

	return QString("0.0.1");
}

Result TrolleyPlugin::execute(const QString& method)
{
	qDebug() << "Trolley执行函数"<<method;
	return Result();
}
