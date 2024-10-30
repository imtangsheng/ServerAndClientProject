#ifndef QTPLUGINDEVICEINTERFACE_H
#define QTPLUGINDEVICEINTERFACE_H

#include <QObject>
#include <QString>
#include <QtPlugin>

QT_BEGIN_NAMESPACE

/**定义一个结构体来包含更详细的结果信息**/
struct Result
{
    bool success;
    QString message;

    Result(bool s = true, const QString &msg = "") : success(s), message(msg) {}
    operator bool() const { return success; } // 重载了 bool 操作符，使其可以像之前的 bool 返回值一样使用例如：if (result)
};

// 定义事件类型
enum class DeviceEvent {
    Initialize,
    Start,
    Pause,
    Resume,
    Stop,
    Error,
    Reset,
    Update,
    Custom
};
// 定义状态
enum class DeviceState {
    Uninitialized,
    Initialized,
    Started,
    Paused,
    Stopped,
    Error,
    Destroyed
};

using StateHandler = std::function<Result(const QVariant&)>;

class IPluginDevice : public QObject
{
    Q_OBJECT

public:
    explicit IPluginDevice(QObject *parent = nullptr) :QObject(parent),
        currentState(DeviceState::Uninitialized) {
        // initializeStateTransitions();
    }
    virtual ~IPluginDevice() = default;

    virtual QString name() const = 0;    // 设备名称
    virtual QString version() const = 0; // 版本
    virtual DeviceState getCurrentState() { return currentState; }

    // 第一次加载初始化,配置等 不用参与任务执行,用于软件第一次启动
    virtual Result awake() = 0; // 始终在任何 start 函数之前并在实例化预制件之后调用此函数。（如果对象在启动期间处于非活动状态，则在激活之后才会调用 Awake。）
    virtual Result start() = 0; // 实例后，在更新之前调用
    virtual void update_fixed() {} // 物理计算和更新
    virtual void update() = 0;      // 更新的主要函数
    virtual void update_late() {}
    virtual Result pause() { return Result(true); }
    virtual void resume() {};  // 恢复
    virtual Result stop() = 0; // 停止
    virtual void destroy() = 0; // 注销
    virtual void executeFeature(const QString &featureName) = 0; // 执行特定功能

    // 处理事件的方法
    Result handleEvent(DeviceEvent event, const QVariant &data = QVariant()) {
        if (stateHandlers.contains(currentState) &&
            stateHandlers[currentState].contains(event)) {
            return stateHandlers[currentState][event](data);
        }
        return Result(false, "Invalid state transition");
    }

signals:
    void stateChanged(DeviceState oldState, DeviceState newState);
    void eventOccurred(DeviceEvent event, const QVariant &data);
    void error(const QString &errorMessage);

protected:
    DeviceState currentState;
    // 状态转换处理器
    QMap<DeviceState, QMap<DeviceEvent, StateHandler>> stateHandlers;
    // 注册自定义事件处理器
    void registerEventHandler(DeviceState state, DeviceEvent event,StateHandler handler) {
        stateHandlers[state][event] = handler;
    }
    virtual void setState(DeviceState newState) {
        if (currentState != newState) {
            DeviceState oldState = currentState;
            currentState = newState;
            emit stateChanged(oldState, newState);
        }
    }

private:
    // 初始化默认状态转换 声明为私有纯虚函数
    virtual void initializeStateTransitions() = 0;
};

// 定义插件接口的唯一标识符
#define QtPluginDeviceInterface_iid "interface.Qt.Plugin.Device/0.1"
Q_DECLARE_INTERFACE(IPluginDevice, QtPluginDeviceInterface_iid)

QT_END_NAMESPACE

#endif // QTPLUGINDEVICEINTERFACE_H
