#include "ManagerPlugin.h"

ManagerPlugin::ManagerPlugin(QObject* parent)
	: QObject(parent)
{
	qDebug() << "ManagerPlugin::ManagerPlugin() 构造函数";
}

ManagerPlugin::~ManagerPlugin()
{
	qDebug() << "ManagerPlugin::~ManagerPlugin() 析构函数";
	// 卸载所有插件
	for (const auto& pluginName : m_plugins.keys()) {
		unloadPlugin(pluginName);
	}
}

Result ManagerPlugin::scanPlugins(const QString& pluginDir)
{
	m_pluginDir = pluginDir;
	if (!m_pluginDir.exists()) {
		return Result::Failure("插件目录不存在");
	}
	// 扫描插件目录
	QStringList filters;
#ifdef Q_OS_WIN
	filters << "*.dll";
#else
	filters << "*.so";
#endif
	plugins_available = m_pluginDir.entryList(filters, QDir::Files);
	qInfo() << "Found" << plugins_available.size() << "plugins in" << pluginDir;

	// 移除文件扩展名
	for (int i = 0; i < plugins_available.size(); ++i) {
		QString& pluginName = plugins_available[i];
		int lastDot = pluginName.lastIndexOf('.');
		if (lastDot > 0) {
			pluginName = pluginName.left(lastDot);
		}
	}
	return Result::Success();
}

Result ManagerPlugin::loadPlugin(const QString& pluginName)
{
	// 检查插件是否已加载
	if (m_plugins.contains(pluginName)) {
		qInfo() << "Plugin already loaded:" << pluginName;
		return true;
	}
	// 构建插件文件路径
	QString pluginFilePath = m_pluginDir.absoluteFilePath(pluginName);
#ifdef Q_OS_WIN
	pluginFilePath += ".dll";
#else
	pluginFilePath += ".so";
#endif
	// 创建插件加载器
	QPluginLoader* loader = new QPluginLoader(pluginFilePath);
	if (!loader->load()) {
		qCritical() << "Failed to load plugin" << pluginName << ":" << loader->errorString();
		delete loader;
		return Result::Failure("Failed to load plugin");
	}
	
	// 获取插件实例
	PluginDeviceInterface* plugin = qobject_cast<PluginDeviceInterface*>(loader->instance());
	if (!plugin) {
		qCritical() << "Failed to get plugin instance:" << pluginName;
		delete loader;
		return Result::Failure("Failed to get plugin instance");
	}
	//加载json文件的元组
	QJsonObject jsonFile;
/**
	当使用 Q_PLUGIN_METADATA 宏并指定 JSON 文件时，Qt 在编译时会将该 JSON 文件的内容嵌入到插件二进制文件中。Qt 使用一个标准化的结构来存储这些元数据：
	顶层结构包含几个预定义的键：
	IID : 插件接口标识符
	className : 插件类名
	MetaData : 包含用户定义的自定义元数据,即json文件的内容
	Keys : 可选的插件键列表
	当调用 loader.metaData() 时，你获取的是整个顶层 JSON 对象
*/
	if (auto metadata = loader->metaData(); !metadata.isEmpty()) {
		QString iid = metadata["IID"].toString();
		if (iid == PluginDeviceInterface_iid) {
			qDebug() << "加载设备的插件:" << PluginDeviceInterface_iid << metadata["className"].toString() << metadata["Keys"].toString();
		}
		jsonFile = metadata.value("MetaData").toObject();
	}
	// 添加插件到列表 存储插件信息
	PluginData pluginData;
	pluginData.loader = loader;
	pluginData.interface = plugin;
	pluginData.json = jsonFile;
	m_plugins.insert(pluginName, pluginData);
	qInfo() << "Plugin loaded:" << pluginName << "version:" << plugin->version() << "name:" << plugin->name();
	plugin->initialize();
	return Result::Success();
}

Result ManagerPlugin::unloadPlugin(const QString& pluginName)
{
	// 检查插件是否已加载
	if (!m_plugins.contains(pluginName)) {
		qWarning() << "Plugin not loaded:" << pluginName;
		return Result::Failure("Plugin not loaded");
	}
	// 获取插件数据
	PluginData pluginData = m_plugins.take(pluginName);

	// 卸载插件
	if (!pluginData.loader->unload()) {
		qWarning() << "Failed to unload plugin" << pluginName << ":" << pluginData.loader->errorString();
		return Result::Failure("Failed to unload plugin");
	}
	//清理资源
	delete pluginData.loader;
	m_plugins.remove(pluginName);
	qInfo() << "Plugin unloaded:" << pluginName;
	return Result::Success();
}

PluginDeviceInterface* ManagerPlugin::getPlugin(const QString& pluginName)
{
	if (!m_plugins.contains(pluginName)) {
		return nullptr;
	}
	return m_plugins[pluginName].interface;
}

