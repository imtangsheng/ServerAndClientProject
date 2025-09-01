/*!
 * @file SerialPlugin.cpp
 * @brief
 * @date 2025-04
 */

#ifdef DEVICE_TYPE_CAR
#pragma message("生成的动态库是小车设备")
#endif
#ifdef DEVICE_TYPE_CAMERA
#pragma message("生成的动态库是相机设备")
#endif
#include "SerialPlugin.h"
#include "public/serial/ActiveSerial.h"

static ActiveSerial* gSerial{ nullptr };// = new ActiveSerial();

SerialPlugin::SerialPlugin() {
    IPluginDevice::initialize();
    qDebug() << "[#Serial]构造函数";
    gShare.RegisterHandler(GetModuleName(), this);
}

SerialPlugin::~SerialPlugin() {
    qDebug() << "[#Serial]析构函数";
}

QString SerialPlugin::GetModuleName() const {
    return g_module_name;
}

Result SerialPlugin::Activate_(QJsonObject param) {
    QString port = param.value("port").toString();
    if(port.isEmpty()){
        LOG_ERROR(tr("缺少必须的串口名参数"));
        return Result::Failure(tr("缺少必须的串口名参数"));
    }
    if (gSerial->isOpen()) {
        if (port == gSerial->GetPortName())//不用重复打开同一个串口
            return Result::Failure(tr("串口已打开,不能重复打开"));
        else 
            gSerial->close();
    }
    Result result = gSerial->open(port);
    if (result) {
        config_["port"] = port;
        WriteJsonFile(ConfigFilePath(), config_);
    }
    return result;
}

Result SerialPlugin::initialize() {
    gSerial = new ActiveSerial(this, config_.value("port").toString());
    QJsonObject general = config_.value("general").toObject();
#ifdef DEVICE_TYPE_CAR
    //读取小车速度乘数
    if (general.contains("mileage_multiplier")) {
        QString key = general["mileage_multiplier"].toString();
        if (MILEAGE_MULTIPLIER.contains(key)) {//是使用了配置的值
            g_mileage_multiplier = MILEAGE_MULTIPLIER.value(key).toDouble();
        } else {
            g_mileage_multiplier = key.toDouble();//自定义的数字
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
    if (MILEAGE_MULTIPLIER.contains(key)) {//是使用了配置的值
        g_mileage_multiplier = MILEAGE_MULTIPLIER.value(key).toDouble();
    } else {
        bool ok;
        double num = key.toDouble(&ok);//自定义的数字
        if (ok) {
            g_mileage_multiplier = num;
        } else {
            gShare.on_send(Result::Failure(tr("you enter a type that is not double, it is invalid")), session);
        }
    }
    {//对配置加锁
        QMutexLocker locker(&_mutex_config);
        QJsonObject general = config_["general"].toObject();
        general["mileage_multiplier"] = key;
        config_["general"] = general;
    }
    Result result = WriteJsonFile(ConfigFilePath(), config_);
    gShare.on_send(result, session);
}
#endif // DEVICE_TYPE_CAR
void SerialPlugin::initUi(const Session& session) {
    QJsonObject obj;
#ifdef DEVICE_TYPE_CAR
    obj["mileage_multiplier"] = MILEAGE_MULTIPLIER.keys().join(",");
#endif // DEVICE_TYPE_CAR
    emit gSigSent(session.ResponseString(obj, tr("succeed")), session.socket);
    //主动请求一次配置更新
    emit gSigSent(Session::RequestString(2, GetModuleName(), "onConfigChanged", QJsonArray{ config_ }), session.socket);
}


void SerialPlugin::execute(const QString& method) {
    qDebug() << "[#Serial]执行方法" << method;
}

void SerialPlugin::scan(const Session& session) {
    gShare.on_session(session.ResponseString(gSerial->GetAvailablePorts().join(",")), session.socket);
}

void SerialPlugin::start(const Session& session) {
    gSerial->start(session);
}

void SerialPlugin::stop(const Session& session) {
    gSerial->stop(session);
}

void SerialPlugin::GetInfoByCode(const Session& session) {
    FunctionCodeType code = static_cast<FunctionCodeType>(session.params.toInt());
    if (!gSerial->WriteData(code)) {
        return gShare.on_send(Result::Failure(tr("命令码:%1执行失败,请确认串口是否已打开或存在其他错误").arg(code)), session);
    }
    g_serial_session.addSession(code, session);
}

void SerialPlugin::SetParameterByCode(const Session& session) {
    QJsonObject obj = session.params.toObject();
    if (obj.contains("code")) {
        FunctionCodeType code = static_cast<FunctionCodeType>(obj.value("code").toInt());
        // 反序列化 使用十六进制字符串
        QString hexData = obj.value("data").toString();
        QByteArray data = QByteArray::fromHex(hexData.toUtf8());
        qDebug() << obj;
        if (!gSerial->WriteData(code, data)) {
            return gShare.on_send(Result::Failure(tr("命令码:%1执行失败,请确认串口是否已打开或存在其他错误").arg(code)), session);
        }
        g_serial_session.addSession(code, session);
    } else {
        gShare.on_send(Result::Failure(tr("没有找到关键字\"code\",请确认命令是否正确")), session);
    }
}

void SerialPlugin::onConfigChanged() const {
    emit gSigSent(Session::RequestString(2, GetModuleName(), "onConfigChanged", QJsonArray{ config_ }));
}

void SerialPlugin::SetConfig(const Session& session) {
    QJsonObject root = session.params.toObject();
    Result result = gSerial->SetConfig(root);
    gShare.on_send(result, session);
}
