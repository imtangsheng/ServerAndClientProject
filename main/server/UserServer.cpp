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
        //dir.mkpath(".");
        QString msg = QObject::tr("#项目:路径不存在%1").arg(path_project);
        LOG_WARNING(msg);
        return Result::Failure(msg);
    }
    //获得只返回当前目录下的所有子目录(不包括 . 和 ..) 的效果
    QStringList projectNameList = dir.entryList(QStringList() << cProjectNameSuffix, QDir::Dirs | QDir::NoDotAndDotDot);
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
    //connect(&gShare, &share::ShareLib::signal_language_changed, this, &UserServer::onLanguageChanged);
    //语言翻译文件加载
    if (!translator.load(":/i18n/" + zh_CN)) {
        LOG_ERROR(tr("加载语言错误:%1").arg(gShare.language));
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

void UserServer::initialize(quint16 port_ws, quint16 port_http) {
    gHttpServer = new HttpServer();
    if (!gHttpServer->StartServer(port_http)) {
        LOG_ERROR(tr("启动HTTP服务失败,端口号:%1").arg(port_http));
    }
    gWebSocketServer = new WebSocketServer(port_ws, this);
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
        session.module = plugin.ptr->GetModuleName();
        session.params = QJsonArray{ {plugin.ptr->state_,"state"} };//double 类型传输值 message用于显示信息
        gShare.on_session(session.GetRequest(), session.socket);
    }
}

void UserServer::SetRealtimeParsing(Session session) {
    bool checked = session.params.toBool();
    gSettings->setValue("RealtimeParsing", checked);
    //gShare.isRealtimeParsing = checked;
    gShare.on_success(tr("设置实时解析"), session);
}

void UserServer::onLanguageChanged(QString language) {
    qDebug() << "UserServer::onLanguageChanged" << language;
    gSettings->setValue("language", language);
    emit gShare.signal_language_changed(language);
}

void UserServer::onAutoStartedClicked(const Session& session) {
    bool checked = session.params.toBool();
    SetAutoStart(checked);//设置开机自启 注册表的方法
    gShare.on_success(tr("设置开机自启"), session);
}

void UserServer::onCarWarningClicked(const Session& session) {
    //需要检查小车模块,然后使用命令设置,目前该功能待完成
    bool checked = session.params.toBool();
    gSettings->setValue("CarWarning", checked);
    gShare.on_success(tr("设置车辆警告"), session);
}

void UserServer::onLogLevelChanged(const Session& session) {
    int level = session.params.toInt();
    gSettings->setValue("LogLevel", level);
    gLog.logLevel = static_cast<LogLevel>(level);
    gShare.on_success(tr("设置日志级别"), session);
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
    gShare.on_session(session.ResponseString(gTaskManager.data, tr("succeed")), session.socket);
}

enum SessionErrorCode : int
{
    SessionErrorNone = 0, //没有错误
    SessionErrorInvalid = 1,//发送了无效的数据
    SessionErrorWorkflow =2, //操作有重复或者顺序错误
};
void UserServer::AddNewProject(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.isEmpty()) {
        return gShare.on_session(session.ErrorString(SessionErrorInvalid, tr("Invalid params data")), session.socket);
    }
    FileInfoDetails project;
    if (!project.FromJsonObject(obj)) {
        return gShare.on_session(session.ErrorString(SessionErrorInvalid, tr("Invalid project json data")), session.socket);
    };
    //判断文件夹是否存在
    QDir dir(project.path);
    if (dir.exists()) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("Directory is exist")), session.socket);
    }
    //判断是否存在同名项目
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    if (projects.contains(project.name)) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("The project to be added already exists.")),session.socket);
    }
    //写入json配置到文件中
    if (!WriteJsonFile(project.path + "/" + kProjectInfoFileName, project.data)) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("Failed to write project data to file")), session.socket);
    }

    projects[project.name] = project.ToJsonObject();
    gTaskManager.data[cKeyContent] = projects;

    return gShare.on_session(session.ResponseString(SessionErrorNone,tr("Add project succeed")), session.socket);
}

void UserServer::DeleteProject(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.isEmpty()) {
        return gShare.on_session(session.ErrorString(SessionErrorInvalid, tr("Invalid params data")), session.socket);
    }
    FileInfoDetails project;
    if (!project.FromJsonObject(obj)) {
        return gShare.on_session(session.ErrorString(SessionErrorInvalid, tr("Invalid project json data")), session.socket);
    };
    //判断文件夹是否存在
    QDir dir(project.path);
    if (!dir.exists()) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("Directory is not exist")), session.socket);
    }
    //判断是否存在同名项目
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    if (!projects.contains(project.name)) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("The project to be deleted does not exist.")), session.socket);
    }
    //删除文件夹
    if (!dir.removeRecursively()) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("Failed to delete directory")), session.socket);
    }
    //删除json配置到文件中
    projects.remove(project.name);
    gTaskManager.data[cKeyContent] = projects;

    return gShare.on_session(session.ResponseString(SessionErrorNone, tr("Delete project succeed")), session.socket);
}

void UserServer::AddTaskAsCurrent(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.isEmpty()) {
        return gShare.on_session(session.ErrorString(SessionErrorInvalid, tr("Invalid params data")), session.socket);
    }
    FileInfoDetails task;
    if (!task.FromJsonObject(obj)) {
        return gShare.on_session(session.ErrorString(SessionErrorInvalid, tr("Invalid project json data")), session.socket);
    };
     //判断文件夹是否存在
    QDir dir(task.path);
    if (dir.exists()) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("Directory is exist")), session.socket);
    }
    //获取任务目录的上一级目录名称就是项目名称
    if (!dir.cdUp()) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow,tr("Task Path No parent directory for %1.").arg(dir.absolutePath())));
    }
    QString projectName = dir.dirName();
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    if (projectName.isEmpty() || !projects.contains(projectName)) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow,tr("The project does not exist: %1.").arg(projectName)));
    }
    QJsonObject project = projects.value(projectName).toObject();
    QJsonObject tasks = project.value(cKeyContent).toObject();
    if (task.name.isEmpty() || tasks.contains(task.name)) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("The task name is empty or the task to be added already exists: %1.").arg(task.name)));
    }

    //写入json配置到文件中
    Result fileResult = WriteJsonFile(task.path + "/" + kTaskInfoFileName, task.ToJsonObject());
    if(fileResult) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, fileResult.message));
    }
    *gTaskFileInfo = task;//设置当前任务
    tasks[task.name] = task.ToJsonObject(); //更新任务
    project[cKeyContent] = tasks;//保存任务
    projects[projectName] = project;//更新项目
    gTaskManager.data[cKeyContent] = projects;//保存项目
    return gShare.on_session(session.ResponseString(SessionErrorNone, tr("add task succeed")), session.socket);
}

void UserServer::DeleteTask(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.isEmpty()) {
        return gShare.on_session(session.ErrorString(SessionErrorInvalid, tr("Invalid params data")), session.socket);
    }
    FileInfoDetails task;
    if (!task.FromJsonObject(obj)) {
        return gShare.on_session(session.ErrorString(SessionErrorInvalid, tr("Invalid project json data")), session.socket);
    };
    //判断文件夹是否存在
    QDir dir(task.path);
    if (!dir.exists()) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("Directory is not exist")), session.socket);
    }
    //获取任务目录的上一级目录名称就是项目名称
    if (!dir.cdUp()) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("Task Path No parent directory for %1.").arg(dir.absolutePath())));
    }
    QString projectName = dir.dirName();
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    if (projectName.isEmpty() || !projects.contains(projectName)) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("The project does not exist: %1.").arg(projectName)));
    }
    QJsonObject project = projects.value(projectName).toObject();
    QJsonObject tasks = project.value(cKeyContent).toObject();
    if (task.name.isEmpty() || tasks.contains(task.name)) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("The task name is empty or the task to be added already exists: %1.").arg(task.name)));
    }
    //删除任务文件夹
    QDir dirTask(task.path);
    if (!dirTask.removeRecursively()) {
        return gShare.on_session(session.ErrorString(SessionErrorWorkflow, tr("DeleteTask is Failed, The task folder removed failed.")));
    }
    tasks.remove(task.name);
    project[cKeyContent] = tasks;
    projects[projectName] = project;//更新项目
    gTaskManager.data[cKeyContent] = projects;//保存项目
    return gShare.on_session(session.ResponseString(SessionErrorNone, tr("delete task succeed")), session.socket);
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
        result = gManagerPlugin->m_plugins[deviceType].ptr->OnStarted();
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
        gManagerPlugin->m_plugins[trolleyString].ptr->OnStopped([session, this](bool success) {
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
                gManagerPlugin->m_plugins[deviceType].ptr->OnStopped();
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
