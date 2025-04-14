/*!
 * @file IPluginDevice.h
 * @brief Qt的插件接口,用于动态加载和卸载设备类 ->cn
 * @date 2025-02-24
 */
#ifndef QTPLUGINDEVICEINTERFACE_H
#define QTPLUGINDEVICEINTERFACE_H
#include"share_lib.h"
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

class IPluginDevice : public QObject
{
	Q_OBJECT
public:
	explicit IPluginDevice(QObject* parent = nullptr) :QObject(parent),
		currentState(DeviceState::Disconnected) {
		qDebug() << "IPluginDevice()构造函数被调用" << QThread::currentThread();
	}
	virtual ~IPluginDevice() {
		qDebug() << "IPluginDevice()析构函数被调用";
	}

	virtual QString _module() const = 0;//记录当前设备模块名称 必要用于注册设备
	double state_{0};//记录当前设备状态值
	QString stateString;//监控显示信息
	// 基本操作接口
	virtual void initialize() {
		if (!translator.load(":/" + _module() + "/" + zh_CN)) {
			LOG_ERROR(tr("Failed to load PluginDevice language file:%1").arg(zh_CN));
		}
		if (gSouth.language == zh_CN) {
			emit gSouth.signal_translator_load(translator, true);
		}
		connect(&gSouth, &south::ShareLib::signal_language_changed, this, [this](const QString& language) {
            if (language == zh_CN) {
                emit gSouth.signal_translator_load(translator, true);
            } else {
				emit gSouth.signal_translator_load(translator, false);
            }
		});

	}

    virtual Result disconnect() = 0; // 断开连接

    // 事件
    virtual Result AcquisitionStart() = 0; // 开始捕获
	virtual Result AcquisitionStop() = 0;

	// 参数设置接口 根据接收到的json数据,自动化设置参数
	virtual Result SetParameters(const QJsonObject& parameters) = 0;

	virtual QString name() const = 0;    // 设备名称
	virtual QString version() const = 0; // 版本
	// 执行约定的方法
	virtual Result execute(const QString& method) = 0; // 执行特定功能

	virtual DeviceState GetCurrentState() { return currentState; }
	// 获取错误信息
	virtual QString GetLastError() {return lastError;};


	// 处理事件的方法
	Result HandleEvent(DeviceEvent event, const QJsonValue& data = QJsonValue()) {
		if (stateHandlers.contains(currentState) &&
			stateHandlers[currentState].contains(event)) {
			return stateHandlers[currentState][event](data);
		}
		return Result(false, "Invalid state transition");
	}

signals:
	void state_changed(DeviceState oldState, DeviceState newState);
	void event_occurred(DeviceEvent event, const QVariant& data);
	void error_occurred(const QString& errorMessage);

protected:
	QString lastError;
	DeviceState currentState;
	// 状态转换处理器
	QMap<DeviceState, QMap<DeviceEvent, StateHandler>> stateHandlers;
	// 注册自定义事件处理器
	void RegisterEventHandler(DeviceState state, DeviceEvent event, StateHandler handler) {
		stateHandlers[state][event] = handler;
	}
	virtual void SetState(DeviceState newState) {
		if (currentState != newState) {
			DeviceState oldState = currentState;
			currentState = newState;
			emit state_changed(oldState, newState);
		}
	} 

    QTranslator translator;//成员变量,用于国际化 必须由qt主线程析构
//protected slots:

};

// 定义插件接口的唯一标识符
#define PluginDeviceInterface_iid "interface.Qt.IPluginDevice/0.1"
Q_DECLARE_INTERFACE(IPluginDevice, PluginDeviceInterface_iid)//使用 Q_DECLARE_INTERFACE 声明接口，以便 Qt 的插件系统识别。

QT_END_NAMESPACE

#endif // QTPLUGINDEVICEINTERFACE_H
