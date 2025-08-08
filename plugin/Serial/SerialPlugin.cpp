/*!
 * @file SerialPlugin.cpp
 * @brief
 * @date 2025-04
 */
#include "SerialPlugin.h"
#include "public/serial/ActiveSerial.h"

static ActiveSerial* gSerial{ nullptr };// = new ActiveSerial();

SerialPlugin::SerialPlugin() {
    IPluginDevice::initialize();
    qDebug() << "[#Serial]构造函数";
    gShare.RegisterHandler(_module(), this);
}

SerialPlugin::~SerialPlugin() {
    qDebug() << "[#Serial]析构函数";
}

QString SerialPlugin::_module() const {
    return g_module_name;
}

Result SerialPlugin::initialize() {
    gSerial = new ActiveSerial(this, config_.value("port").toString());
    QJsonObject general = config_.value("general").toObject();
#ifdef DEVICE_TYPE_CAR
    //读取小车速度乘数
    if (general.contains("speed_multiplier")) {
        QString key = general["speed_multiplier"].toString();
        if (SPEED_MULTIPLIER.contains(key)) {//是使用了配置的值
            speed_multiplier = SPEED_MULTIPLIER.value(key).toDouble();
        } else {
            speed_multiplier = key.toDouble();//自定义的数字
        }
    }

#endif // DEVICE_TYPE_CAR
    return true;
}

Result SerialPlugin::disconnect() {
    qDebug() << "[#Serial]断开连接";
    return Result();
}


QString SerialPlugin::name() const {
    //return gSerial->DeviceName();
    return "Serial";
}

QString SerialPlugin::version() const {
    return QString("0.0.1");
}

Result SerialPlugin::OnStarted(CallbackResult callback) {
    return gSerial->OnStarted(callback);
}

Result SerialPlugin::OnStopped(CallbackResult callback) {
    return gSerial->OnStopped(callback);
}

#ifdef DEVICE_TYPE_CAR
void SerialPlugin::SetSpeedMultiplier(const Session& session) {
    QString key = session.params.toString();
    if (SPEED_MULTIPLIER.contains(key)) {//是使用了配置的值
        speed_multiplier = SPEED_MULTIPLIER.value(key).toDouble();
    } else {
        bool ok;
        double num = key.toDouble(&ok);//自定义的数字
        if (ok) {
            speed_multiplier = num;
        } else {
            gShare.on_send(Result::Failure(tr("you enter a type that is not double, it is invalid")), session);
        }
    }
    {//对配置加锁
        QMutexLocker locker(&_mutex_config);
        QJsonObject general = config_["general"].toObject();
        general["speed_multiplier"] = key;
        config_["general"] = general;
    }
    Result result = WriteJsonFile(ConfigFilePath(), config_);
    gShare.on_send(result, session);
}
#endif // DEVICE_TYPE_CAR
void SerialPlugin::initUi(const Session& session) {
    QJsonObject obj;
#ifdef DEVICE_TYPE_CAR
    obj["speed_multiplier"] = SPEED_MULTIPLIER.keys().join(",");
#endif // DEVICE_TYPE_CAR
    emit gSigSent(session.ResponseString(obj, tr("succeed")), session.socket);
    //主动请求一次配置更新
    emit gSigSent(Session::RequestString(2, _module(), "onConfigChanged", QJsonArray{ config_ }), session.socket);
}


void SerialPlugin::execute(const QString& method) {
    qDebug() << "[#Serial]执行方法" << method;
}

void SerialPlugin::scan(const Session& session) {
    emit gSigSent(session.ResponseString(gSerial->GetAvailablePorts().join(","), tr("succeed")), session.socket);
}

void SerialPlugin::open(const Session& session) {
    QString port = session.params.toString();
    if (port.isEmpty()) {
        emit gSigSent(session.ErrorString(-1, tr("port is empty")), session.socket);
        return;
    }
    Result result = gSerial->open(port);
    gShare.on_send(result, session);
}

void SerialPlugin::start(const Session& session) {
    gSerial->start(session);
}

void SerialPlugin::stop(const Session& session) {
    gSerial->stop(session);
}

void SerialPlugin::GetInfoByCode(const Session& session) {
    FunctionCodeType code = static_cast<FunctionCodeType>(session.params.toInt());
    gSerial->WriteData(code);
    g_serial_session.addSession(code, session);
}

void SerialPlugin::SetParamsByCode(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.contains("code")) {
        FunctionCodeType code = static_cast<FunctionCodeType>(obj["code"].toInt());
        QByteArray data = obj["data"].toString().toUtf8();
        gSerial->WriteData(code, data);
        g_serial_session.addSession(code, session);
    } else {
        gShare.on_send(Result::Failure(tr("The key value \"code\" does not exist in the of JSON the Params")), session);
    }
}

void SerialPlugin::onConfigChanged() const {
    emit gSigSent(Session::RequestString(2, _module(), "onConfigChanged", QJsonArray{ config_ }));
}

void SerialPlugin::SetParams(const Session& session) {
    QJsonObject root = session.params.toObject();
    Result result = gSerial->SetConfig(root);
    gShare.on_send(result, session);
}
