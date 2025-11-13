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
    //获得只返回当前目录下的所有子目录(不包括 . 和 ..) 的效果QStringList() << cProjectNameSuffix
    QStringList projectNameList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    // 过滤出符合后缀的目录
    static QRegularExpression regex(QString(".*%1$").arg(cProjectNameSuffix));
    projectNameList = projectNameList.filter(regex);
    //更新工作目录 信息
    data[cKeyPath] = path_project;
    QJsonObject objProjects;
    for (auto& projectDir : projectNameList) {
        //添加项目
        QString absoluteProjectPath = dir.absoluteFilePath(projectDir);//获取项目绝对路径
        QString projectFilePath = absoluteProjectPath + "/" + kProjectInfoFileName;
        QJsonObject project;//项目 json数据
        project[cKeyName] = projectDir;
        project[cKeyPath] = absoluteProjectPath;
        QJsonObject obj;
        if (!GetJsonValue(projectFilePath, obj)) {
            continue;//读取或者解析 json失败
        }
        project[cKeyData] = obj;
        //项目任务处理
        QDir dirTask(absoluteProjectPath);
        QStringList taskNameList = dirTask.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        QJsonObject objTasks;
        for (auto& taskDir : taskNameList) {
            //添加任务
            QString absoluteTaskPath = dirTask.absoluteFilePath(taskDir);//获取任务绝对路径
            QString taskFilePath = absoluteTaskPath + "/" + kTaskInfoFileName;
            QJsonObject task_obj;
            if (!GetJsonValue(taskFilePath, task_obj)) {
                continue;
            }
            QJsonObject objTask;
            objTask[cKeyName] = taskDir;
            objTask[cKeyPath] = absoluteTaskPath;
            objTask[cKeyData] = task_obj;
            objTasks[taskDir] = objTask;//使用文件名作为唯一 key
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

    QString path = gShare.RegisterSettings->value(TASK_KEY_DATAPAHT,gShare.appPath+"/data").toString();
    UpdateProjectDir(path, gTaskManager.data); //更新项目信息

    gManagerPlugin = new ManagerPlugin(this);
    if (gManagerPlugin->PluginsScan()) {
        for (QString& pluginName : gManagerPlugin->pluginsAvailable) {
            gManagerPlugin->PluginLoad(pluginName);//加载插件
        }
    }
}

void UserServer::onTest() {
    //QString filepath = "../data/唐.nfproj/test/PointCloud/Scan001.fls";
    //QStringList args = { "-f", filepath };
    //QString exePath = gShare.appPath + "/bin/FaroPreview.exe";
    //qDebug() << "启动预览:" << exePath << args;
    //gShare.shellProcess(exePath, args);
}

void UserServer::onDeviceStateChanged(const Session &session) {
    qDebug() << "UserServer::onDeviceStateChanged";
    for (auto& plugin : gManagerPlugin->plugins) {
        gShare.on_session(Session::RequestString(plugin.ptr->GetModuleName(),
            session.method ,QJsonArray{ plugin.ptr->state_.toDouble() }), session.socket);//double 类型传输值 message用于显示信息
    }
}

void UserServer::SetRealtimePreview(Session session) {
    bool checked = session.params.toBool();
    gSettings->setValue("IsRealtimePreview", checked);
    gShare.info["IsRealtimePreview"] = checked;
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
#include<QSerialPortInfo>
void UserServer::GetAvailableSerialPorts(const Session& session) {
    QStringList ports;
    foreach(const QSerialPortInfo & info, QSerialPortInfo::availablePorts()) {
        ports << info.portName();
    }
    gShare.on_session(session.ResponseSuccess(ports.join(",")), session.socket);
}

void UserServer::GetTaskData(const Session& session) {
    qDebug() << "获取项目任务数据库:" << gTaskManager.data;
    gShare.on_session(session.ResponseSuccess(gTaskManager.data), session.socket);
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
        return gShare.on_session(session.Finished(SessionErrorInvalid, tr("无效的参数类型")), session.socket);
    }
    FileInfoDetails project;
    if (!project.FromJsonObject(obj)) {
        return gShare.on_session(session.Finished(SessionErrorInvalid, tr("无效的项目参数类型")), session.socket);
    };
    //判断文件夹是否存在
    QDir dir(project.path);
    if (dir.exists()) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("项目目录已经存在")), session.socket);
    }
    //判断是否存在同名项目
    QJsonObject projects = gTaskManager.GetProjects();
    if (SafeJsonHasKey(projects.keys(),project.name)) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("项目名称已经存在,请更换名称.")),session.socket);
    }
    //写入 json 配置到文件中
    if (!WriteJsonFile(project.path + "/" + kProjectInfoFileName, project.data)) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("项目写入配置文件失败")), session.socket);
    }

    projects[project.name] = project.ToJsonObject();
    gTaskManager.data[cKeyContent] = projects;

    return gShare.on_session(session.ResponseSuccess(SessionErrorNone,tr("创建新项目成功")), session.socket);
}

void UserServer::DeleteProject(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.isEmpty()) {
        return gShare.on_session(session.Finished(SessionErrorInvalid, tr("无效的参数类型")), session.socket);
    }
    FileInfoDetails project;
    if (!project.FromJsonObject(obj)) {
        return gShare.on_session(session.Finished(SessionErrorInvalid, tr("无效的项目参数类型")), session.socket);
    };
    //判断文件夹是否存在
    QDir dir(project.path);
    if (!dir.exists()) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("目录不存在")), session.socket);
    }
    //判断是否存在同名项目
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    if (!SafeJsonHasKey(projects.keys(),project.name)) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("要操作的项目名称不存在,请确认名称.")), session.socket);
    }
    //删除文件夹
    if (!dir.removeRecursively()) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("删除文件失败")), session.socket);
    }
    //删除 json配置到文件中
    projects.remove(project.name);
    gTaskManager.data[cKeyContent] = projects;

    return gShare.on_session(session.ResponseSuccess(SessionErrorNone, tr("删除项目成功")), session.socket);
}

void UserServer::AddCurrentTask(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.isEmpty()) {
        return gShare.on_session(session.Finished(SessionErrorInvalid, tr("无效的参数类型")), session.socket);
    }
    FileInfoDetails task;
    if (!task.FromJsonObject(obj)) {
        return gShare.on_session(session.Finished(SessionErrorInvalid, tr("无效的任务参数类型")), session.socket);
    };
     //判断文件夹是否存在
    QDir dir(task.path);
    if (dir.exists()) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("任务名称已经存在,请重新命名")), session.socket);
    }
    //获取任务目录的上一级目录名称就是项目名称
    if (!dir.cdUp()) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow,tr("找不到项目目录和名称:%1.").arg(dir.absolutePath())));
    }
    QString projectName = dir.dirName();
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    if (projectName.isEmpty() || !SafeJsonHasKey(projects.keys(),projectName)) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow,tr("找不到对应的项目名称: %1.").arg(projectName)));
    }
    QJsonObject project = projects.value(projectName).toObject();
    QJsonObject tasks = project.value(cKeyContent).toObject();
    if (task.name.isEmpty() || SafeJsonHasKey(tasks.keys(), task.name)) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("任务名称已经存在: %1.").arg(task.name)));
    }

    //写入 json配置到文件中
    Result fileResult = WriteJsonFile(task.path + "/" + kTaskInfoFileName, task.data);
    if(!fileResult) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, fileResult.message));
    }
    gTaskFileInfo = new FileInfoDetails(task);//设置当前任务
    tasks[task.name] = task.ToJsonObject(); //更新任务
    project[cKeyContent] = tasks;//保存任务
    projects[projectName] = project;//更新项目
    gTaskManager.data[cKeyContent] = projects;//保存项目

    gTaskState = TaskState::TaskState_PreStart; //任务预配置,创建目录,监控等
    return gShare.on_session(session.ResponseSuccess(SessionErrorNone, tr("添加任务成功")), session.socket);
}

void UserServer::DeleteTask(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.isEmpty()) {
        return gShare.on_session(session.Finished(SessionErrorInvalid, tr("无效参数类型")), session.socket);
    }
    FileInfoDetails task;
    if (!task.FromJsonObject(obj)) {
        return gShare.on_session(session.Finished(SessionErrorInvalid, tr("无效的项目参数类型")), session.socket);
    };
    //判断文件夹是否存在
    QDir dir(task.path);
    if (!dir.exists()) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("目录不存在")), session.socket);
    }
    //获取任务目录的上一级目录名称就是项目名称
    if (!dir.cdUp()) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("找不到项目目录%1.").arg(dir.absolutePath())));
    }
    QString projectName = dir.dirName();
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    if (projectName.isEmpty() || !SafeJsonHasKey(projects.keys(), projectName)) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("项目目录不存在: %1.").arg(projectName)));
    }
    QJsonObject project = projects.value(projectName).toObject();
    QJsonObject tasks = project.value(cKeyContent).toObject();
    if (task.name.isEmpty() || !SafeJsonHasKey(tasks.keys(),task.name)) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("任务不存:在%1.").arg(task.name)));
    }
    //删除任务文件夹
    QDir dirTask(task.path);
    if (!dirTask.removeRecursively()) {
        return gShare.on_session(session.Finished(SessionErrorWorkflow, tr("删除任务目录文件失败.")));
    }
    tasks.remove(task.name);
    project[cKeyContent] = tasks;
    projects[projectName] = project;//更新项目
    gTaskManager.data[cKeyContent] = projects;//保存项目
    return gShare.on_session(session.ResponseSuccess(SessionErrorNone, tr("删除任务成功")), session.socket);
}


void UserServer::onStart(const Session& session, bool isContinue) {
    qDebug() << "UserServer::onStart" << isContinue;
    // 定义设备顺序 1.相机 2.扫描仪 3.其他
    static const QStringList START_ORDER = {
        share::Shared::GetModuleName(share::ModuleName::camera),
        share::Shared::GetModuleName(share::ModuleName::scanner)
    };
    static QStringList order_devices;// 按顺序重新排列设备列表
    static QStringList started_devices_all;//已经启动的设备
    if (!isContinue) {
        order_devices.clear(); started_devices_all.clear();
        gTaskManager.setState(TaskState::TaskState_Running);
        emit gTaskManager.running();
        QStringList devicesList = gManagerPlugin->plugins.keys();
        for (const auto& device : START_ORDER) {
            if (devicesList.contains(device)) {
                order_devices << device;
                devicesList.removeOne(device);
            }
        }
        // 添加剩余设备
        order_devices << devicesList;
    }
    if(order_devices.isEmpty()) {//完成,返回成功
        gShare.on_send(Result::Success(), session);
        return;
    }
    // 按照预定义顺序执行
    QString device = order_devices.takeFirst();
    gManagerPlugin->plugins[device].ptr->OnStarted([=](const qint8& code, const QJsonValue& value) {
        qDebug() <<"按照预定义顺序执行"<<device << "code:" << code << value;
        if (Result(code)) {
            this->onStart(session,true); //启动后,再继续执行
            started_devices_all << device;
        } else {
            gShare.on_session(session.Finished(code, value.toString()), session.socket);
            gManagerPlugin->stop(started_devices_all);//停止已经启动的设备
        }
    });
}

void UserServer::onStop(const Session& session, bool isContinue) {
    qDebug() << "UserServer::onStop" << isContinue;
    // 定义设备停止顺序 1.小车 2.扫描仪 3.相机
    static const QStringList STOP_ORDER = {
    share::Shared::GetModuleName(share::ModuleName::serial),
    share::Shared::GetModuleName(share::ModuleName::scanner),
    share::Shared::GetModuleName(share::ModuleName::camera)
    };
    static QStringList order_devices;// 按顺序重新排列设备列表
    if (!isContinue) {
        
        order_devices.clear();
        QStringList devices = gManagerPlugin->plugins.keys();
        for (const auto& device : STOP_ORDER) {
            if (devices.contains(device)) {
                order_devices << device;
                devices.removeOne(device);
            }
        }
        // 添加剩余设备
        order_devices << devices;
    }
    if (order_devices.isEmpty()) {//完成,返回成功
        gShare.on_send(Result::Success(), session);
        gTaskManager.setState(TaskState_Finished);
        emit gTaskManager.finished();
        return;
    }
    // 按照预定义顺序执行
    QString device = order_devices.takeFirst();
    gManagerPlugin->plugins[device].ptr->OnStopped([session, this](const qint8& code, const QJsonValue& value) {
        if (Result(code)) {
            this->onStop(session,true); //启动后,再继续执行
        } else {
            gShare.on_session(session.Finished(code, value.toString()), session.socket);
        }
    });
}

#include <QProcess>
void UserServer::shutdown(const Session& session) {
    //先 扫描仪关机,后电脑关机
    if (gManagerPlugin->plugins.keys().contains(sModuleScanner)) {
        Result result = gManagerPlugin->plugins[sModuleScanner].ptr->Shutdown();
        if(!result) {
            gShare.on_session(session.Finished(result.code, tr("扫描仪关机失败,错误码: %1").arg(result.code)), session.socket);
            return;
        }
    }

    // 等待3秒，让其他操作完成
    //QThread::msleep(3000);
    QString command;
    QStringList arguments;
#ifdef Q_OS_WIN
    command = "shutdown";
    arguments << "-s" << "-t" << "0";
#elif defined(Q_OS_MAC)
    command = "sudo";
    arguments << "shutdown" << "-h" << "now";
#elif defined(Q_OS_LINUX)
    command = "sudo";
    arguments << "shutdown" << "now";
#else
    gShare.on_session(session.Finished(-1, tr("不支持的操作系统")), session.socket);
    return;
#endif
    // 尝试多种关机方式
    QProcess process;
    process.start(command, arguments);

    if (!process.waitForStarted()) {
        // 第一种方式失败，尝试直接使用 shutdown
        process.start("shutdown", arguments);
                if (!process.waitForStarted()) {
#ifdef Q_OS_WIN
            // Windows下最后尝试使用 system32下的 shutdown
            process.start("C:/Windows/System32/shutdown.exe", arguments);
            if (!process.waitForStarted()) {
                gShare.on_session(session.Finished(-1, tr("关机命令执行失败，请检查权限")), session.socket);
                return;
            }
#else
            gShare.on_session(session.Finished(-1, tr("关机命令执行失败，请检查权限")), session.socket);
            return;
#endif
        }
    }

    // 等待命令执行完成
    process.waitForFinished(3000);

    // 检查执行结果
    if (process.exitCode() != 0) {
        QString errorMsg = tr("关机命令执行失败: %1").arg(QString::fromLocal8Bit(process.readAllStandardError()));
        gShare.on_session(session.Finished(-1, errorMsg), session.socket);
        return;
    }
    gShare.on_success(tr("正在关机..."), session);
    QCoreApplication::quit();
}
