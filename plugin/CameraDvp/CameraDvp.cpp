/*!
 * @file CameraDvp.cpp
 * @brief 度申相机DVP2(Digital Videl Platform 2) SDK 的API开发的实现文件->cn
 * @date 2025-02
 */
#include "pch.h"
#include "CameraDvp.h"
CameraDvp::CameraDvp()
{
	qDebug() << "CameraDvp构造函数";
	//initialize();
}

CameraDvp::~CameraDvp()
{
	qDebug() << "CameraDvp析构函数";
}

Result CameraDvp::initialize()
{
	qDebug() << "CameraDvp初始化函数";
	
	//qDebug() <<"#测试:"<<&gController<<QThread::currentThreadId();
	gCameraController = new CameraController(this);
	//south::South::instance().registerHandler("camera", gCameraController.data());
	return Result();
}

Result CameraDvp::connect()
{
	qDebug() << "CameraDvp连接函数";
	return Result();
}

Result CameraDvp::disconnect()
{
	qDebug() << "CameraDvp断开函数";
	return Result();
}

Result CameraDvp::startCapture()
{
	qDebug() << "CameraDvp开始采集函数";
	return Result();
}

Result CameraDvp::stopCapture()
{
	qDebug() << "CameraDvp停止采集函数";
	return Result();
}

Result CameraDvp::setParameters(const QJsonObject& parameters)
{
	qDebug() << "CameraDvp设置参数函数"<<parameters;
	return Result();
}

QString CameraDvp::name() const
{
	return QString("CameraDvp");
}

QString CameraDvp::version() const
{

	return QString("0.0.1");
}

Result CameraDvp::execute(const QString& method)
{
	qDebug() << "CameraDvp执行函数"<<method;
	return Result();
}
