/*!
 * @file CameraDvp.h
 * @brief 相机类插件的声明文件->cn
 * @date 2025-02
 */
#include "PluginDeviceInterface.h"

class CameraDvp : public PluginDeviceInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginDeviceInterface_iid FILE "CameraPlugin.json") //QPluginLoader 类来加载插件并读取元数据
    Q_INTERFACES(PluginDeviceInterface)
public:
	CameraDvp();
	~CameraDvp() override;
	// 基本操作接口
	Result initialize()override;
	Result connect() override; // 连接到特定设备
	Result disconnect() override;
	Result startCapture()override;
	Result stopCapture()override;

	// 参数设置接口 根据接收到的json数据,自动化设置参数
	Result setParameters(const QJsonObject& parameters) override;

	QString name() const override;    // 设备名称
	QString version() const override; // 版本
	// 执行约定的方法
	Result execute(const QString& method) override; // 执行特定功能



private:
    /* data */
};