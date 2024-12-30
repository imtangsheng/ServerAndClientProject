#include <QCoreApplication>
#include <QDir>
#include <QPluginLoader>
#include "interface/QtPluginDeviceInterface.h"

class PluginManager
{
public:
    static PluginManager &instance()
    {
        static PluginManager manager;
        return manager;
    }
    // 加载插件目录中的所有插件
    void loadPlugins()
    {
        QDir pluginsDir(QCoreApplication::applicationDirPath() + "/plugins");

        // 遍历插件目录
        for (const QString &fileName : pluginsDir.entryList(QDir::Files))
        {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));

            // 获取插件元数据
            QJsonObject metadata = loader.metaData().value("MetaData").toObject();

            // 创建插件实例
            QObject *plugin = loader.instance();
            if (plugin)
            {
                // 尝试转换为插件接口
                IPluginDevice *interface = qobject_cast<IPluginDevice *>(plugin);
                if (interface)
                {
                    // 初始化插件
                    if (interface->awake())
                    {
                        m_plugins[interface->name()] = interface;
                        qDebug() << "Loaded plugin:" << interface->name()
                                 << "Version:" << interface->version();
                    }
                    else
                    {
                        qWarning() << "Failed to initialize plugin:" << interface->name();
                        loader.unload();
                    }
                }
            }
            else
            {
                qWarning() << "Failed to load plugin:" << fileName
                           << "Error:" << loader.errorString();
            }
        }
    }

    // 获取已加载的插件列表
    QList<IPluginDevice *> plugins() const
    {
        return m_plugins.values();
    }

    // 通过名称获取插件
    IPluginDevice *getPlugin(const QString &name)
    {
        return m_plugins.value(name, nullptr);
    }

private:
    PluginManager() {}
    QMap<QString, IPluginDevice *> m_plugins;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "start test";
    PluginManager::instance().loadPlugins();

    // 遍历所有已加载的插件
    for (IPluginDevice *plugin : PluginManager::instance().plugins())
    {
        qDebug() << "\nPlugin Information:";
        qDebug() << "Name:" << plugin->name();
        qDebug() << "Version:" << plugin->version();
        // qDebug() << "Features:" << plugin->features();

        // // 执行每个插件的所有功能
        // for (const QString& feature : plugin->features()) {
        //     plugin->executeFeature(feature);
        // }
        plugin->handleEvent(DeviceEvent::Custom);
    }

    for (IPluginDevice *device : PluginManager::instance().plugins())
    {
        // 初始化
        if (!device->handleEvent(DeviceEvent::Initialize)) {
            qDebug() << "Initialization failed";
        }
        // 启动
        if (!device->handleEvent(DeviceEvent::Start)) {
            qDebug() << "Start failed";
            continue;
        }

        // 发送自定义事件
        QJsonObject jsonObj;
        jsonObj["name"] = "Alice";
        jsonObj["age"] = 30;

        QVariant jsonVariant = QVariant::fromValue(jsonObj);
        device->handleEvent(DeviceEvent::Custom, jsonVariant);

        // 暂停
        device->handleEvent(DeviceEvent::Pause);

        // 恢复
        device->handleEvent(DeviceEvent::Resume);

        // 停止
        device->handleEvent(DeviceEvent::Stop);
    }
    return app.exec();
}
