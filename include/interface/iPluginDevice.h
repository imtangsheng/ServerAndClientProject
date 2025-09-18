/*!
 * @file IPluginDevice.h
 * @brief Qt的插件接口,用于动态加载和卸载设备类,对应IWidget.h文件 ->cn
 * @date 2025-02-24
 */
#ifndef QTPLUGINDEVICEINTERFACE_H
#define QTPLUGINDEVICEINTERFACE_H
#include"shared.h"
#include "logger.h"
 //#include <QtPlugin>
QT_BEGIN_NAMESPACE

class IPluginDevice : public QObject
{
    Q_OBJECT
public:
    explicit IPluginDevice(QObject* parent = nullptr) :QObject(parent) {
        qDebug() << "[IPluginDevice]构造函数被调用" << QThread::currentThread();
    }
    virtual ~IPluginDevice() {
        qDebug() << "IPluginDevice()析构函数被调用";
    }

    virtual QString GetModuleName() const = 0;//记录当前设备模块名称 必要用于注册设备

    StateEvent state_;//记录设备状态值,int类型,double类型用于 json数据网络传输
    QJsonObject config_;//设备配置参数 json格式
    TaskState taskState{ TaskState::TaskState_Waiting };//任务状态

    Q_INVOKABLE virtual Result Activate_(QJsonObject param) { return true; } //激活设备,注册服务 或者重新激活
    // 基本操作接口 默认设计的是首先要执行一次,执行初始化变量赋值等操作
    virtual Result initialize() {
        if (!translator.load(":/" + name() + "/" + zh_CN)) {
            LOG_ERROR(tr("加载插件语言失败:%1").arg(": / " + name() + " / " + zh_CN));
        }
        if (gShare.language == zh_CN) {
            emit gShare.signal_translator_load(translator, true);
        }
        connect(&gShare, &share::Shared::signal_language_changed, this, [this](const QString& language) {
            if (language == zh_CN) {
                emit gShare.signal_translator_load(translator, true);
            } else {
                emit gShare.signal_translator_load(translator, false);
            }
            });

        return LoadConfigFromFile(name() + ".json");//加载配置文件到 params_

        //注册设备控制模块 该为设备验证后注册服务
        //return gShare.RegisterHandler(GetModuleName(), this);
    }

    virtual Result disconnect() = 0; // 断开连接
    virtual QString name() const = 0;    // 设备名称
    virtual QString version() const = 0; // 版本

    // 事件 直接调用
    virtual Result OnStarted(CallbackResult callback = nullptr) =0;
    virtual Result OnStopped(CallbackResult callback = nullptr) =0;

    // 获取错误信息
    virtual QString GetLastError() { return lastError; };
    virtual Result Shutdown() { return true; }//设备关机目前只有扫描仪有
public slots:
    virtual void initUi(const Session& session) = 0;//初始化UI,返回配置信息
    virtual void SaveConfig(const Session& session) {//保存配置
        config_ = session.params.toObject();
        Result result = WriteJsonFile(ConfigFilePath(), config_);
        if (!result) {
            LOG_WARNING(tr("%1 保存参数错误:%2").arg(name(),result.message));
        }
        gShare.on_send(result, session);
    }
    // 执行约定的方法
    virtual void execute(const QString& method) = 0; // 执行特定功能

signals:
    void error_occurred(const QString& errorMessage);

protected:
    QString lastError;
    QTranslator translator;//成员变量,用于国际化 必须由qt主线程析构
    QString ConfigFilePath() const {
        static QString filePath = gShare.appPath + "/config/" + name() + ".json";
        return filePath;
    }
    Result LoadConfigFromFile(const QString& jsonFilePath) {
        Result result;
        QString absolutePath;
        result = gShare.FindFilePath(jsonFilePath, absolutePath);
        if (!result || absolutePath.isEmpty()) {
            LOG_WARNING(result.message);
            return result;
        }
        //获取配置文件的json数据
        result = ReadJsonFile(absolutePath, config_);
        if (!result || config_.isEmpty()) {
            LOG_WARNING(result.message);
            return result;
        }
        return true;
    }
    //protected slots:

};

// 定义插件接口的唯一标识符
#define PluginDeviceInterface_iid "interface.Qt.IPluginDevice/0.1"
Q_DECLARE_INTERFACE(IPluginDevice, PluginDeviceInterface_iid)//使用 Q_DECLARE_INTERFACE 声明接口，以便 Qt 的插件系统识别。

QT_END_NAMESPACE

#endif // QTPLUGINDEVICEINTERFACE_H
