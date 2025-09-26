 /*!
 * @file ScannerPlugin.h
 * @brief 扫描仪类插件的声明文件,未使用 #pragma once 防止重复包含
 * @date 2025-02
 */
#include "iPluginDevice.h"

class ScannerPlugin : public IPluginDevice
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginDeviceInterface_iid FILE "ScannerPlugin.json") //QPluginLoader 类来加载插件并读取元数据
    Q_INTERFACES(IPluginDevice)
public:
	ScannerPlugin();
	~ScannerPlugin() override;
    // 基本数据定义
    QString GetModuleName() const final;    // 设备名称
    // 基本操作接口
    Q_INVOKABLE Result Activate_(QJsonObject param) final;//激活设备,注册加载初始化等操作
    Result initialize() final;
    Result disactivate() final;
    QString name() const final;    // 设备名称
    QString version() const final; // 版本

    //直接调用 使用回调函数在执行特定顺序的任务
    Result OnStarted(const CallbackResult& callback = nullptr) final;
    Result OnStopped(const CallbackResult& callback = nullptr) final;

    Q_INVOKABLE Result Shutdown() final;
private:
    Result TryConnect();
    void CheckConnect();

public slots:
    void onUpdateUi(const Session& session) final;//初始化UI,返回配置信息
    // 执行约定的方法
    void execute(const QString& method) final; // 执行特定功能
    
    //操作方法
    void ScanConnect(const Session& session);
    void SetParameter(const Session& session);
    void ScanStart(const Session& session);
    void ScanRecord(const Session& session);//只支持螺旋扫描 HelicalCANGrey, and not with HelicalGrey
    void ScanPause(const Session& session);//只支持螺旋扫描 HelicalCANGrey, and not with HelicalGrey
    void ScanStop(const Session& session);
    void GetProgressPercent(const Session& session);
    
    void GetCameraPositionDistance(const Session& session);//计算相机的机位到物理的距离值,即焦距值计算

    void GetSerialNumber(const Session& session); //获取更新扫描仪序列号

};