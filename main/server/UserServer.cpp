#include "UserServer.h"
#include "network/WebSocketServer.h"
#include "manager/ManagerPlugin.h"

static QPointer<WebSocketServer> gWebSocketServer;
UserServer::UserServer(QObject* parent): QObject(parent) {
    gSouth.RegisterHandler(sModuleUser, this);
    
}

UserServer::~UserServer() {
    qDebug() << "UserServer::~UserServer()";
};

void UserServer::initialize(quint16 port) {
    gWebSocketServer = new WebSocketServer(port, this);
    gWebSocketServer->initialize();
    //connect(gWebSocketServer, &WebSocketServer::closed, &app, &QCoreApplication::quit);

	gManagerPlugin = new ManagerPlugin(this);
	if (gManagerPlugin->PluginsScan()) {
		for (QString& pluginName : gManagerPlugin->pluginsAvailable) {
			//是否是无效插件
			if (gManagerPlugin->pluginsInvalid.contains(pluginName)) continue;
			gManagerPlugin->PluginLoad(pluginName);
		}
	}
}

void UserServer::acquisition_begins(const Session& session) {
    qDebug() << "UserServer::acquisition_begins" << session.params;
    //gManagerPlugin
    Result result;
    QStringList devices = gManagerPlugin->m_plugins.keys();
    QStringList devicesStarted;
    if (devices.isEmpty()) {
        gSouth.on_send(Result::Failure("No device found"), session);
        return;
    }
    // 定义设备启动顺序
    const QStringList START_ORDER = { sModuleCamera, sModuleScanner };
    // 按照预定义顺序启动特定设备
    for (const auto& deviceType : START_ORDER) {
        if (!devices.contains(deviceType)) {continue;}
        result = gManagerPlugin->m_plugins[deviceType].interface->AcquisitionStart();
        if (!result) {
            gManagerPlugin->stop(devicesStarted);
            gSouth.on_send(result, session);
            return;
        }

        devicesStarted << deviceType;
        devices.removeOne(deviceType);
    }
    //#3最后启动小车等其他
    result = gManagerPlugin->start(devices);
    gSouth.on_send(result, session);
}

void UserServer::acquisition_end(const Session& session) {
    qDebug() << "UserServer::acquisition_end" << session.params;
    QStringList devices = gManagerPlugin->m_plugins.keys();
    // 定义设备停止顺序
    const QStringList STOP_ORDER = { sModuleTrolley, sModuleScanner, sModuleCamera };
    // 按照预定义顺序停止设备
    for (const auto& deviceType : STOP_ORDER) {
        if (devices.contains(deviceType)) {
            gManagerPlugin->m_plugins[deviceType].interface->AcquisitionStop();
        }
    }
    Result result = gManagerPlugin->stop(devices);
    gSouth.on_send(result, session);
}

void UserServer::shutdown() {
    emit closed();
}
