/*!
 * @file ScannerPlugin.cpp
 * @brief
 * @date 2025-02
 */
#include "pch.h"
#include"ScannerPlugin.h"
ScannerPlugin::ScannerPlugin()
{
	qDebug() << "Scanner构造函数";
	//initialize();
}

ScannerPlugin::~ScannerPlugin()
{
	qDebug() << "Scanner析构函数";
}

Result ScannerPlugin::initialize()
{
	qDebug() << "Scanner初始化函数";
	
	//qDebug() <<"#测试:"<<&gController<<QThread::currentThreadId();
	gScannerController = new ScannerController(this);
	//south::Share::instance().registerHandler("camera", gCameraController.data());
	return Result();
}

Result ScannerPlugin::connect()
{
	qDebug() << "Scanner连接函数";
	return Result();
}

Result ScannerPlugin::disconnect()
{
	qDebug() << "Scanner断开函数";
	return Result();
}

Result ScannerPlugin::AcquisitionStart()
{
	qDebug() << "Scanner开始采集函数";
	return Result();
}

Result ScannerPlugin::AcquisitionStop()
{
	qDebug() << "Scanner停止采集函数";
	return Result();
}

Result ScannerPlugin::SetParameters(const QJsonObject& parameters)
{
	qDebug() << "Scanner设置参数函数"<<parameters;
	return Result();
}

QString ScannerPlugin::name() const
{
	return QString("ScannerPlugin");
}

QString ScannerPlugin::version() const
{

	return QString("0.0.1");
}

Result ScannerPlugin::execute(const QString& method)
{
	qDebug() << "Scanner执行函数"<<method;
	return Result();
}
