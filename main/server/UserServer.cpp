#include "UserServer.h"
#include "network/HttpServer.h"
#include "network/WebSocketServer.h"
#include "manager/ManagerPlugin.h"

#include "public/utils/windows_utils.h"

static QPointer<HttpServer> gHttpServer;
static QPointer<WebSocketServer> gWebSocketServer;

//更新获取项目任务信息
static Result UpdateProjectDir(const QString& path_project, QJsonObject& data) {
    QDir dir(path_project);
    if (!dir.exists()) {
        QString msg = QObject::tr("#TaskManager:Directory does not exist:%1").arg(path_project);
        LOG_WARNING(msg);
        return Result::Failure(msg);
    }
    //获得只返回当前目录下的所有子目录(不包括 . 和 ..) 的效果
    QStringList projectNameList = dir.entryList(QStringList() << kProjectNameSuffix, QDir::Dirs | QDir::NoDotAndDotDot);
    //更新工作目录 信息
    data[cKeyPath] = path_project;
    QJsonObject objProjects;
    for (auto& projectDir : projectNameList) {
        //添加项目
        QString absiluteProjectPath = dir.absoluteFilePath(projectDir);//获取项目绝对路径
        QString projectFilePath = absiluteProjectPath + "/" + kProjectInfoFileName;
        QJsonObject project;//项目json数据
        project[cKeyName] = projectDir;
        project[cKeyPath] = absiluteProjectPath;
        QJsonObject obj;
        if (!GetJsonValue(projectFilePath, obj)) {
            continue;//读取或者解析json失败
        }
        project[cKeyData] = obj;
        //项目任务处理
        QDir dirTask(absiluteProjectPath);
        QStringList taskNameList = dirTask.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        QJsonObject objTasks;
        for (auto& taskDir : taskNameList) {
            //添加任务
            QString absiluteTaskPath = dirTask.absoluteFilePath(taskDir);//获取任务绝对路径
            QString taskFilePath = absiluteTaskPath + "/" + kTaskInfoFileName;
            QJsonObject task_obj;
            if (!GetJsonValue(taskFilePath, task_obj)) {
                continue;
            }
            QJsonObject objTask;
            objTask[cKeyName] = taskDir;
            objTask[cKeyPath] = absiluteTaskPath;
            objTask[cKeyData] = task_obj;
            objTasks[taskDir] = objTask;//使用文件名作为唯一key
        }
        project[cKeyContent] = objTasks;
        objProjects[projectDir] = project;//存入
    }
    data[cKeyContent] = objProjects;
    return true;
};

UserServer::UserServer(QObject* parent) : QObject(parent) {
    module_ = share::Shared::GetModuleName(share::ModuleName::user);
    gShare.RegisterHandler(module_, this);
    //connect(&gShare, &share::ShareLib::signal_language_changed, this, &UserServer::onLanguageChanged);
    //语言翻译文件加载
    if (!translator.load(":/i18n/" + zh_CN)) {
        LOG_ERROR(tr("Failed to load language:%1").arg(gShare.language));
    }
    if (gShare.language == zh_CN) {
        emit gShare.signal_translator_load(translator, true);
    }
    connect(&gShare, &share::Shared::signal_language_changed, this, [this](const QString& language) {
        if (language == zh_CN) {
            emit gShare.signal_translator_load(translator, true);
        } else {
            emit gShare.signal_translator_load(translator, false);
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

    QString path = gShare.RegisterSettings->value(TASK_KEY_DATAPAHT,gShare.appPath+"/data").toString();
    UpdateProjectDir(path, gTaskManager.data); //更新项目信息

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
    emit gShare.signal_language_changed(language);
}

void UserServer::onAutoStartedClicked(bool checked) {
    SetAutoStart(checked);//设置开机自启 注册表的方法
}

void UserServer::SetRegisterSettings(const Session& session) {
    qDebug() << "UserServer::SetRegisterSettings" << session.params;
    //设置注册表 key value
    QJsonObject reg = session.params.toObject();
    for (auto it = reg.begin(); it != reg.end(); ++it) {
        QString key = it.key();
        QVariant value = it.value().toVariant();
        gShare.RegisterSettings->value(key, value);
    }
    gShare.on_send(Result::Success(), session);
}

void UserServer::GetTaskData(const Session& session) {
    emit gSigSent(session.ResponseString(gTaskManager.data, tr("succeed")), session.socket);
}

void UserServer::SetCurrentTask(const Session& session) {
    QJsonObject task = session.params.toObject();
    gTask->name = task[cKeyName].toString();
    gTask->path = task[cKeyPath].toString();
    gTask->data = task[cKeyData].toObject();

    //可移动到开始任务信息那里
    gShare.on_send(gTaskManager.AddTask(task), session);
}


void UserServer::acquisition_begins(const Session& session) {
    qDebug() << "UserServer::acquisition_begins" << session.params;
    //gManagerPlugin
    Result result;
    QStringList devices = gManagerPlugin->m_plugins.keys();
    QStringList devicesStarted;
    if (devices.isEmpty()) {
        gShare.on_send(Result::Failure("No device found"), session);
        return;
    }
    // 定义设备启动顺序 1.相机 2.扫描仪 3.其他
    static const QStringList START_ORDER = {
        share::Shared::GetModuleName(share::ModuleName::camera),
        share::Shared::GetModuleName(share::ModuleName::scanner)
    };
    // 按照预定义顺序启动特定设备
    for (const auto& deviceType : START_ORDER) {
        if (!devices.contains(deviceType)) { continue; }
        result = gManagerPlugin->m_plugins[deviceType].self->OnStarted();
        if (!result) {
            gManagerPlugin->stop(devicesStarted);
            gShare.on_send(result, session);
            return;
        }

        devicesStarted << deviceType;
        devices.removeOne(deviceType);
    }
    //#3最后启动小车等其他
    result = gManagerPlugin->start(devices);
    gShare.on_send(result, session);
}

void UserServer::acquisition_end(const Session& session) {
    qDebug() << "UserServer::acquisition_end" << session.params;
    // 定义设备停止顺序 1.小车 2.扫描仪 3.相机
    static const QStringList STOP_ORDER = {
    share::Shared::GetModuleName(share::ModuleName::scanner),
    share::Shared::GetModuleName(share::ModuleName::camera)
    };
    //定义小车是否已经停止过了
    static bool isTrolleyStopped = false;
    //插件设备列表
    QStringList devices = gManagerPlugin->m_plugins.keys();
    static const auto trolleyString = share::Shared::GetModuleName(share::ModuleName::serial);
    if (!isTrolleyStopped && devices.contains(trolleyString)) {
        gManagerPlugin->m_plugins[trolleyString].self->OnStopped([session, this](bool success) {
            if (success) {
                isTrolleyStopped = true;
                this->acquisition_end(session);
            } else {
                gShare.on_send(Result::Failure("Failed to stop trolley"), session);
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
        gShare.on_send(result, session);
    }
}

void UserServer::shutdown() {
    emit closed();
}
