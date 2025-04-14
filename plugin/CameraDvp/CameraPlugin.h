/*!
 * @file CameraPlugin.h
 * @brief 相机类插件的声明文件,未使用 #pragma once 防止重复包含
 * @date 2025-02
 */
#include "iPluginDevice.h"

class CameraPlugin : public IPluginDevice
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginDeviceInterface_iid FILE "CameraPlugin.json") //QPluginLoader 类来加载插件并读取元数据
    Q_INTERFACES(IPluginDevice)
public:
	CameraPlugin();
	~CameraPlugin() override;

	// 基本数据定义
	QString _module() const override;    // 设备名称

	// 基本操作接口
	void initialize()override;
	Result disconnect() override;
	Result AcquisitionStart()override;
	Result AcquisitionStop()override;

	// 参数设置接口 根据接收到的json数据,自动化设置参数
	Result SetParameters(const QJsonObject& parameters) override;

	QString name() const override;    // 设备名称
	QString version() const override; // 版本
	// 执行约定的方法
	Result execute(const QString& method) override; // 执行特定功能
private:
    /* data */
};