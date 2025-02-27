/*!
 * @file PluginDeviceInterface.h
 * @brief Qt的插件接口,用于动态加载和卸载设备类 ->cn
 * @date 2025-02-24
 */
#ifndef QTPLUGINDEVICEINTERFACE_H
#define QTPLUGINDEVICEINTERFACE_H
#include"./SouthGlobal.h"
//#include <QtPlugin>

QT_BEGIN_NAMESPACE

// 定义设备事件类型
enum DeviceEvent {
	Initialize, Start, Pause, Resume, Stop, Error, Reset, Update, Custom
};
// 定义设备的状态
enum DeviceState {
	Disconnected, Connected, Capturing
};

using StateHandler = std::function<Result(const QJsonValue&)>;

class PluginDeviceInterface : public QObject
{
	Q_OBJECT
public:
	explicit PluginDeviceInterface(QObject* parent = nullptr) :QObject(parent),
		currentState(DeviceState::Disconnected) {
		qDebug() << "PluginDeviceInterface()构造函数被调用";
		initialize();
	}
	virtual ~PluginDeviceInterface() {
		qDebug() << "PluginDeviceInterface()析构函数被调用";
	}

	// 基本操作接口
	virtual Result initialize() {return Result(true);}
	virtual Result connect() = 0; // 连接到特定设备
	virtual Result disconnect() = 0;
	virtual Result startCapture() = 0;
	virtual Result stopCapture() = 0;

	// 参数设置接口 根据接收到的json数据,自动化设置参数
	virtual Result setParameters(const QJsonObject& parameters) = 0;

	virtual QString name() const = 0;    // 设备名称
	virtual QString version() const = 0; // 版本
	// 执行约定的方法
	virtual Result execute(const QString& method) = 0; // 执行特定功能

	virtual DeviceState getCurrentState() { return currentState; }
	// 获取错误信息
	virtual QString getLastError() {return lastError;};


	// 处理事件的方法
	Result handleEvent(DeviceEvent event, const QJsonValue& data = QJsonValue()) {
		if (stateHandlers.contains(currentState) &&
			stateHandlers[currentState].contains(event)) {
			return stateHandlers[currentState][event](data);
		}
		return Result(false, "Invalid state transition");
	}

signals:
	void stateChanged(DeviceState oldState, DeviceState newState);
	void eventOccurred(DeviceEvent event, const QVariant& data);
	void error(const QString& errorMessage);

protected:
	QString lastError;
	DeviceState currentState;
	// 状态转换处理器
	QMap<DeviceState, QMap<DeviceEvent, StateHandler>> stateHandlers;
	// 注册自定义事件处理器
	void registerEventHandler(DeviceState state, DeviceEvent event, StateHandler handler) {
		stateHandlers[state][event] = handler;
	}
	virtual void setState(DeviceState newState) {
		if (currentState != newState) {
			DeviceState oldState = currentState;
			currentState = newState;
			emit stateChanged(oldState, newState);
		}
	}
};

// 定义插件接口的唯一标识符
#define PluginDeviceInterface_iid "interface.Qt.PluginDeviceInterface/0.1"
Q_DECLARE_INTERFACE(PluginDeviceInterface, PluginDeviceInterface_iid)//使用 Q_DECLARE_INTERFACE 声明接口，以便 Qt 的插件系统识别。

QT_END_NAMESPACE

#endif // QTPLUGINDEVICEINTERFACE_H
