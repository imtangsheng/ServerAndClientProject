#pragma once
#include <QDir>
#include<QPluginLoader>
#include "PluginDeviceInterface.h"
/**
 * @brief 插件管理器类
 * 负责加载、卸载和管理设备插件
 */
class ManagerPlugin : public QObject {
    Q_OBJECT

public:
    explicit ManagerPlugin(QObject* parent = nullptr);
    ~ManagerPlugin();

    QStringList plugins_available;          // 可用的插件列表

    /**
     * @brief 初始化插件系统，扫描插件目录
     * @param pluginDir 插件目录路径
     * @return 是否初始化成功
     */
    Result scanPlugins(const QString& pluginDir = "../plugins");

    /**
     * @brief 加载指定插件
     * @param pluginName 插件名称
     * @return 是否加载成功
     */
    Result loadPlugin(const QString& pluginName);

    /**
     * @brief 卸载指定插件
     * @param pluginName 插件名称
     * @return 是否卸载成功
     */
    Result unloadPlugin(const QString& pluginName);

    /**
     * @brief 获取插件接口
     * @param pluginName 插件名称
     * @return 插件接口指针
     */
    PluginDeviceInterface* getPlugin(const QString& pluginName);

private:
    struct PluginData {
        QPluginLoader* loader;
        PluginDeviceInterface* interface;
        QJsonObject json;
    };

    QDir m_pluginDir;                        // 插件目录
    QMap<QString, PluginData> m_plugins;     // 已加载的插件映射
   
};

inline QPointer<ManagerPlugin> gManagerPlugin = nullptr;
