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

public:
    explicit ManagerPlugin(QObject* parent = nullptr);
    ~ManagerPlugin();

    QStringList pluginsAvailable;          // 可用的插件列表
    QStringList pluginsInvalid;//无效的插件列表

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
private:
    struct PluginData {
        QPluginLoader* loader{nullptr};
        IPluginDevice* self{ nullptr };
        QJsonObject json;
    };

    QDir m_pluginDir;                        // 插件目录
    QMap<QString, PluginData> m_plugins;     // 已加载的插件映射
    friend class UserServer;
   
};

inline QPointer<ManagerPlugin> gManagerPlugin = nullptr;
