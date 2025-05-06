#include "UserServer.h"
#include "network/HttpServer.h"
#include "network/WebSocketServer.h"
#include "manager/ManagerPlugin.h"

#include "public/utils/windows_utils.h"

static QPointer<HttpServer> gHttpServer;
static QPointer<WebSocketServer> gWebSocketServer;

UserServer::UserServer(QObject* parent): QObject(parent) {
    module_ = south::Shared::GetModuleName(south::ModuleName::user);
    gSouth.RegisterHandler(module_, this);
    //connect(&gSouth, &south::ShareLib::signal_language_changed, this, &UserServer::onLanguageChanged);
    //语言翻译文件加载
    if (!translator.load(":/i18n/" + zh_CN)) {
        LOG_ERROR(tr("Failed to load language:%1").arg(gSouth.language));
    }
    if (gSouth.language == zh_CN) {
        emit gSouth.signal_translator_load(translator, true);
    }
    connect(&gSouth, &south::Shared::signal_language_changed, this, [this](const QString& language) {
        if (language == zh_CN) {
            emit gSouth.signal_translator_load(translator, true);
        } else {
            emit gSouth.signal_translator_load(translator, false);
        }
    });
    qDebug() << tr("UserServer Init");
}

UserServer::~UserServer() {
    qDebug() << "UserServer::~UserServer()";
};

void UserServer::initialize(quint16 port) {
    gHttpServer = new HttpServer();
    if (!gHttpServer->StartServer(80)) {
        LOG_ERROR(tr("Failed to start HTTP server"));
    }
    gWebSocketServer = new WebSocketServer(port, this);
    gWebSocketServer->initialize();
    //connect(gWebSocketServer, &WebSocketServer::closed, &app, &QCoreApplication::quit);

	gManagerPlugin = new ManagerPlugin(this);
	if (gManagerPlugin->PluginsScan()) {
		for (QString& pluginName : gManagerPlugin->pluginsAvailable) {
			//是否是无效插件
			if (gManagerPlugin->pluginsInvalid.contains(pluginName)) continue;
            gManagerPlugin->PluginLoad(pluginName);//加载插件
		}
	}
}


void UserServer::onDeviceStateChanged(Session session) {
    qDebug() << "UserServer::onDeviceStateChanged";
    for (auto& plugin : gManagerPlugin->m_plugins) {
        session.module = plugin.interface->_module();
        session.params = QJsonArray{ {plugin.interface->state_,"state"} };
        emit gSigSent(session.getRequest(), session.socket);
    }
}

void UserServer::onLanguageChanged(QString language) {
    qDebug() << "UserServer::onLanguageChanged" << language;
    gSettings->setValue("language", language);
    emit gSouth.signal_language_changed(language);
}

void UserServer::onAutoStartedClicked(bool checked) {
    SetAutoStart(checked);//设置开机自启 注册表的方法
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
    // 定义设备启动顺序 1.相机 2.扫描仪 3.其他
    static const QStringList START_ORDER = { 
        south::Shared::GetModuleName(south::ModuleName::camera),
        south::Shared::GetModuleName(south::ModuleName::scanner)
    };
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
    // 定义设备停止顺序 1.小车 2.扫描仪 3.相机
    static const QStringList STOP_ORDER = {
        south::Shared::GetModuleName(south::ModuleName::trolley),
        south::Shared::GetModuleName(south::ModuleName::scanner),
        south::Shared::GetModuleName(south::ModuleName::camera)
    };
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
