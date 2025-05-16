/*!
 * @file SerialPlugin.h
 * @brief 插件的声明文件,未使用 #pragma once 防止重复包含
 * @date 2025-02
 */
#include "iPluginDevice.h"

class SerialPlugin : public IPluginDevice
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginDeviceInterface_iid FILE "SerialPlugin.json") //QPluginLoader 类来加载插件并读取元数据
    Q_INTERFACES(IPluginDevice)
public:
	SerialPlugin();
	~SerialPlugin() override;
    // 基本数据定义
    QString _module() const override;    // 设备名称
    // 基本操作接口
    void initialize() override;
    Result disconnect() override;
    QString name() const override;    // 设备名称
    QString version() const override; // 版本

    //直接调用 使用回调函数在执行特定顺序的任务
    Result OnStarted(CallbackResult callback = nullptr) final;
    Result OnStopped(CallbackResult callback = nullptr) final;

#ifdef DEVICE_TYPE_CAR
    //#属性设置接口
    Q_INVOKABLE void SetSpeedMultiplier(const Session& session); // 设置小车速度乘数
#endif // DEVICE_TYPE_CAR
public slots:
    void initUi(const Session& session) final;//初始化UI,返回配置信息
    void SaveConfig(const Session& session) final;
    // 执行约定的方法
    void execute(const QString& method) final; // 执行特定功能
    //interface IControllerSDK
    //速度 1.直接C++调用	1x	最快，
    //2.Q_INVOKABLE通过QMetaObject::invokeMethod	约10-20x	比直接调用慢
    //3.slot通过信号触发	约25-40x	最慢的调用方式
    //#SDK接口方法
    void scan(const Session& session);
    void open(const Session& session);
    void start(const Session& session);
    void stop(const Session& session);
    
    void GetInfoByCode(const Session& session);//根据指令获取信息
    void SetParamsByCode(const Session& session);//设置小车参数
    //#界面操作接口
    void onConfigChanged() const;
    void SetParams(const Session& session);

private:
    /* data */
    QMutex _mutex_config;
};
