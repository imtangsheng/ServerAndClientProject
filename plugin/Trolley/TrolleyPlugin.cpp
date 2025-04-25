/*!
 * @file TrolleyPlugin.cpp
 * @brief
 * @date 2025-04
 */
#include "TrolleyPlugin.h"

#include "trolley_control.h"

static TrolleyControl* gControl = new TrolleyControl();

static QString g_plugin_module_name = south::ShareLib::GetModuleName(south::ModuleName::trolley);
TrolleyPlugin::TrolleyPlugin() {
    qDebug() << "[#Trolley]构造函数";
    IPluginDevice::initialize();

    QJsonObject general = config_.value("general").toObject();
    //读取小车速度乘数
    if (general.contains("speed_multiplier")) {
        QString key = general["speed_multiplier"].toString();
        if (gControl->speed_multiplier_json.contains(key)) {//是使用了配置的值
            speed_multiplier = gControl->speed_multiplier_json[key].toDouble();
        } else {
            speed_multiplier = key.toDouble();//自定义的数字
        }
    }


}

TrolleyPlugin::~TrolleyPlugin() {
    qDebug() << "[#Trolley]析构函数";
}

QString TrolleyPlugin::_module() const {
    return g_plugin_module_name;
}

void TrolleyPlugin::initialize() {
    if (!gControl->initialize()) {
        qWarning() << "[#Trolley]初始化失败";
    };
}

Result TrolleyPlugin::disconnect() {
    qDebug() << "[#Trolley]断开连接";
    return Result();
}


QString TrolleyPlugin::name() const {
    return gControl->DeviceName();
}

QString TrolleyPlugin::version() const {
    return QString("0.0.1");
}

void TrolleyPlugin::SetSpeedMultiplier(const Session& session) {
    QString key = session.params.toString();
    if (gControl->speed_multiplier_json.contains(key)) {//是使用了配置的值
        speed_multiplier = gControl->speed_multiplier_json[key].toDouble();
    } else {
        bool ok;
        double num = key.toDouble(&ok);//自定义的数字
        if (ok) {
            speed_multiplier = num;
        } else {
            gSouth.on_send(Result::Failure(tr("you enter a type that is not double, it is invalid")), session);
        }
    }
    {//对配置加锁
        QMutexLocker locker(&_mutex_config);
        QJsonObject general = config_["general"].toObject();
        general["speed_multiplier"] = key;
        config_["general"] = general;
    }
    Result result = gSouth.WriteJsonFile(ConfigFilePath(), config_);
    gSouth.on_send(result, session);
}

void TrolleyPlugin::initUi(const Session& session) {
    QJsonObject obj;
    obj["speed_multiplier"] = gControl->speed_multiplier_json.keys().join(",");

    emit gSigSent(session.ResponseString(obj, tr("succeed")), session.socket);
    //主动请求一次配置更新
    emit gSigSent(Session::RequestString(2, _module(), "onConfigChanged", QJsonArray{ config_ }), session.socket);
}

void TrolleyPlugin::SaveConfig(const Session& session) {
    config_ = session.params.toObject();
    Result result = gSouth.WriteJsonFile(ConfigFilePath(), config_);
    if (!result) {
        LOG_WARNING(tr("Save params error:%1").arg(result.message));
    }
    gSouth.on_send(result, session);
}

void TrolleyPlugin::execute(const QString& method) {
    qDebug() << "[#Trolley]执行方法" << method;
}

Result TrolleyPlugin::AcquisitionStart() {
    return gControl->start();
}

Result TrolleyPlugin::AcquisitionStop() {
    return gControl->stop();
}

void TrolleyPlugin::scan(const Session& session) {
    Result result = gControl->scan();
    if (result) {
        QString device_names = gControl->device_id_list.join(",");
        emit gSigSent(session.ResponseString(device_names, tr("succeed")), session.socket);
    } else {
        emit gSigSent(session.ResponseString(result.code, tr("failed")), session.socket);
    }
}

void TrolleyPlugin::open(const Session& session) {
    Result result = gControl->open();
    gSouth.on_send(result, session);
}

void TrolleyPlugin::start(const Session& session) {
    Result result = gControl->start();
    gSouth.on_send(result, session);
}

void TrolleyPlugin::stop(const Session& session) {
    Result result = gControl->stop();
    gSouth.on_send(result, session);
}

void TrolleyPlugin::onConfigChanged() const {
    emit gSigSent(Session::RequestString(2, _module(), "onConfigChanged", QJsonArray{ config_ }));
}

void TrolleyPlugin::SetParams(const Session& session) {
    QJsonObject root = session.params.toObject();
    Result result = gControl->SetConfig(root);
    gSouth.on_send(result, session);
}
