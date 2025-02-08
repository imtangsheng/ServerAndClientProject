#ifndef PLUGINDEMO_H
#define PLUGINDEMO_H

#include <QJsonObject>
#include "interface/QtPluginDeviceInterface.h"

// 定义插件接口的唯一标识符
#define iid_Test "interface.Qt.Plugin.Device.Test/0.1"
// QtPluginDeviceInterface_iid
class PluginDemo : public IPluginDevice
{
    Q_OBJECT //需要被包含,moc自动生成中
    Q_PLUGIN_METADATA(IID QtPluginDeviceInterface_iid FILE "demo.json")
    Q_INTERFACES(IPluginDevice)

public:
    explicit PluginDemo(QObject *parent = nullptr) : IPluginDevice(parent)
    {
        // 注册自定义事件处理器
        registerEventHandler(DeviceState::Started, DeviceEvent::Custom,
                             [&](const QVariant& data) -> Result {
                                // 处理自定义事件
                                QJsonObject newJsonObj = data.value<QJsonObject>();
                                // 输出 JSON 对象的内容
                                qDebug() << "Name: " << newJsonObj["name"].toString();

                                qDebug() << "PluginDemo Custom event handled with data:" << data;
                                return Result(true, "PluginDemo Custom event processed");
                             });

        initializeStateTransitions();
        // 连接状态变化信号
        connect(this, &PluginDemo::stateChanged,this, &PluginDemo::onStateChanged);
        // 连接错误信号
        connect(this, &PluginDemo::error,this, &PluginDemo::onError);
    }

    virtual ~PluginDemo();

    virtual QString name() const override;    // 设备名称
    virtual QString version() const override; // 版本

    // 第一次加载初始化,配置等 不用参与任务执行,用于软件第一次启动
    virtual Result awake() override;   // 始终在任何 start 函数之前并在实例化预制件之后调用此函数。（如果对象在启动期间处于非活动状态，则在激活之后才会调用 Awake。）
    virtual Result start() override; // 实例后，在更新之前调用

    virtual void update() override;       // 更新的主要函数

    virtual Result stop() override; // 停止

    void destroy() final; // 注销

    virtual void executeFeature(const QString& featureName) override; // 执行特定功能
// signals:
//     virtual void stateChanged(State newState) override;

private slots:
    void onStateChanged(DeviceState oldState, DeviceState newState) {
        qDebug() << "State changed from" << static_cast<int>(oldState)
        << "to" << static_cast<int>(newState);
    }

    void onError(const QString& errorMessage) {
        qDebug() << "Error occurred:" << errorMessage;
    }

private:
    QJsonObject m_metadata;        // 插件元数据
    bool initialized = false;

    void initializeStateTransitions() final {
        // 实现具体的状态转换逻辑
        // Uninitialized -> Initialized
        stateHandlers[DeviceState::Uninitialized][DeviceEvent::Initialize] =
            [this](const QVariant&) -> Result {
            Result result = this->awake();
            if (result) {
                setState(DeviceState::Initialized);
            }
            return result;
        };

        // Initialized -> Started
        stateHandlers[DeviceState::Initialized][DeviceEvent::Start] =
            [this](const QVariant&) -> Result {
            Result result = this->start();
            if (result) {
                setState(DeviceState::Started);
            }
            return result;
        };

        // Started -> Paused
        stateHandlers[DeviceState::Started][DeviceEvent::Pause] =
            [this](const QVariant&) -> Result {
            Result result = this->pause();
            if (result) {
                setState(DeviceState::Paused);
            }
            return result;
        };

        // Paused -> Started
        stateHandlers[DeviceState::Paused][DeviceEvent::Resume] =
            [this](const QVariant&) -> Result {
            this->resume();
            setState(DeviceState::Started);
            return Result(true);
        };

        // Any -> Stopped
        auto stopHandler = [this](const QVariant&) -> Result {
            Result result = this->stop();
            if (result) {
                setState(DeviceState::Stopped);
            }
            return result;
        };

        stateHandlers[DeviceState::Started][DeviceEvent::Stop] = stopHandler;
        stateHandlers[DeviceState::Paused][DeviceEvent::Stop] = stopHandler;

        // Error handling for all states
        auto errorHandler = [this](const QVariant& data) -> Result {
            setState(DeviceState::Error);
            emit error(data.toString());
            return Result(false, data.toString());
        };

        for (int state = 0; state <= static_cast<int>(DeviceState::Destroyed); ++state) {
            stateHandlers[static_cast<DeviceState>(state)][DeviceEvent::Error] = errorHandler;
        }
    }
};

#endif // PLUGINDEMO_H
