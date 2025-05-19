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
        session.module = plugin.self->_module();
        session.params = QJsonArray{ {plugin.self->state_,"state"} };//double 类型传输值 message用于显示信息
        emit gSigSent(session.GetRequest(), session.socket);
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

void UserServer::SetRegisterSettings(const Session& session) {
    qDebug() << "UserServer::SetRegisterSettings" << session.params;
    //设置注册表
    QJsonObject reg = session.params.toObject();
    for (auto it = reg.begin(); it != reg.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value().toVariant();
        gSouth.RegisterSettings->value(key, value);
    }
    gSouth.on_send(Result::Success(), session);
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
        result = gManagerPlugin->m_plugins[deviceType].self->OnStarted();
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
    // 定义设备停止顺序 1.小车 2.扫描仪 3.相机
    static const QStringList STOP_ORDER = {
    south::Shared::GetModuleName(south::ModuleName::scanner),
    south::Shared::GetModuleName(south::ModuleName::camera)
    };
    //定义小车是否已经停止过了
    static bool isTrolleyStopped = false;
    //插件设备列表
    QStringList devices = gManagerPlugin->m_plugins.keys();
    static const auto trolleyString = south::Shared::GetModuleName(south::ModuleName::serial);
    if (!isTrolleyStopped && devices.contains(trolleyString)) {
        gManagerPlugin->m_plugins[trolleyString].self->OnStopped([session,this](bool success) {
            if (success) {
                isTrolleyStopped = true;
                this->acquisition_end(session);
            } else {
                gSouth.on_send(Result::Failure("Failed to stop trolley"), session);
            }
        });
    } else {
        // 按照预定义顺序停止设备
        for (const auto& deviceType : STOP_ORDER) {
            if (devices.contains(deviceType)) {
                gManagerPlugin->m_plugins[deviceType].self->OnStopped();
            }
        }
        Result result = gManagerPlugin->stop(devices);
        isTrolleyStopped = false;//重新开始计算
        gSouth.on_send(result, session);
    }
}

void UserServer::shutdown() {
    emit closed();
}
