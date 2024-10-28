#ifndef PLUGINDEMO_H
#define PLUGINDEMO_H

#include <QJsonObject>
#include "interface/QtPluginDeviceInterface.h"

// 定义插件接口的唯一标识符
#define iid_Test "interface.Qt.Plugin.Device.Test/0.1"
// QtPluginDeviceInterface_iid
class PluginDemo : public PluginInterfaceDevice
{
    Q_OBJECT //需要被包含,moc自动生成中
    Q_PLUGIN_METADATA(IID QtPluginDeviceInterface_iid FILE "demo.json")
    Q_INTERFACES(PluginInterfaceDevice)

public:
    explicit PluginDemo(QObject *parent = nullptr);
    virtual ~PluginDemo();

    virtual QString name() const override;    // 设备名称
    virtual QString version() const override; // 版本

    // 第一次加载初始化,配置等 不用参与任务执行,用于软件第一次启动
    virtual Result awake() override;   // 始终在任何 start 函数之前并在实例化预制件之后调用此函数。（如果对象在启动期间处于非活动状态，则在激活之后才会调用 Awake。）
    virtual Result start() override; // 实例后，在更新之前调用

    virtual void update() override;       // 更新的主要函数

    virtual Result stop() override; // 停止

    void destroy(); // 注销

    virtual void executeFeature(const QString& featureName) override; // 执行特定功能
// signals:
//     virtual void stateChanged(State newState) override;

private:
    QJsonObject m_metadata;        // 插件元数据
};

#endif // PLUGINDEMO_H
