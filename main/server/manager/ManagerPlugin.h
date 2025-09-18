#pragma once
#include <QDir>
#include<QPluginLoader>
#include<QStringList>
#include<QPointer>
#include "iPluginDevice.h"
/**
 * @brief 插件管理器类
 * 负责加载、卸载和管理设备插件
 */
class ManagerPlugin : public QObject {
    Q_OBJECT
private:
    struct PluginData {
        QPluginLoader* loader{ nullptr };//插件的加载器,便于加载和卸载
        IPluginDevice* ptr{ nullptr };//插件的指针引用
        QJsonObject json;//插件信息
    };
public:
    explicit ManagerPlugin(QObject* parent = nullptr);
    ~ManagerPlugin();

    QStringList pluginsAvailable;          // 有效的插件列表
    QStringList pluginsInvalid;//无效的插件列表
    QMap<QString, PluginData> plugins;     // 已加载的插件映射

    Result start(QStringList& pluginsName);
    Result stop(QStringList& pluginsName);

    /**
     * @brief 初始化插件系统，扫描插件目录
     * @param pluginDir 插件目录路径
     * @return 是否初始化成功
     */
    Result PluginsScan(const QString& pluginDir = "../plugins");

    /**
     * @brief 加载指定插件
     * @param pluginName 插件名称
     * @return 是否加载成功
     */
    Result PluginLoad(const QString& pluginName);

    /**
     * @brief 卸载指定插件
     * @param pluginName 插件名称
     * @return 是否卸载成功
     */
    Result PluginUnload(const QString& pluginName);

    /**
     * @brief 获取插件接口
     * @param pluginName 插件名称
     * @return 插件接口指针
     */
    IPluginDevice* PluginGetPtr(const QString& pluginName);

public slots:
    void switch_plugin(const QString& pluginName,const bool& enable = false);
    void Activate(const Session& session);
private:
    QDir m_pluginDir;                        // 插件目录

   
};

inline QPointer<ManagerPlugin> gManagerPlugin = nullptr;
