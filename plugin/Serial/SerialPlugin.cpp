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
    if (gSerial->open(port)) {
        config_["port"] = port;
        WriteJsonFile(ConfigFilePath(), config_);
        return true;
    }
    return Result::Failure(tr("打开串口%1失败,请检查是否被其他应用程序占用").arg(port));
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

void SerialPlugin::ScanAutomationTimeSync(const Session& session) {
    static const int time_sync_interval = 30 * 1000; // 30秒
    static QTimer timer(gSerial);
    if (ScannerAndCarTimeInfo::isAwake) {//要先判断是否在开机后进行设置了自动化时间同步,目前设计客户端也会实现一次同步
        bool enable = session.params.toBool(true);
        if (enable) {
            gSerial->WriteData(SCANER_GET_AUTOMATION_TIME);
            g_serial_session.addSession(SCANER_GET_AUTOMATION_TIME, session);
            //if(timer.isActive() == false) //是否激活启动过的判断
            connect(&timer, &QTimer::timeout, this, []() {// 每次调用都会添加新连接
                gSerial->WriteData(SCANER_GET_AUTOMATION_TIME);
                });
            // 确保唯一连接 Qt::UniqueConnection 不能用于 lambda 表达式或非成员函数
            connect(&gTaskManager, &TaskManager::finished, &timer, &QTimer::stop, Qt::UniqueConnection);
            timer.start(time_sync_interval);
        } else {
            timer.disconnect();
            timer.stop();
            gShare.on_success(tr("关闭成功"), session);
        }
    } else {
        gSerial->WriteData(AUTOMATION_TIME_SYNC);
        g_serial_session.addSession(AUTOMATION_TIME_SYNC, [this, session](const qint8& code, const QJsonValue& value) {
            if (code == RESULT_SUCCESS) {
                ScanAutomationTimeSync(session);
            } else {
                gShare.on_session(session.Finished(code, value.toString()), session.socket);
            }
            });
    }
}
#endif // DEVICE_TYPE_CAR
void SerialPlugin::initUi(const Session& session) {
    QJsonObject obj;
#ifdef DEVICE_TYPE_CAR
    obj["mileage_multiplier"] = MILEAGE_MULTIPLIER.keys().join(",");
#endif // DEVICE_TYPE_CAR
    emit gSigSent(session.ResponseSuccess(obj, tr("succeed")), session.socket);
    //主动请求一次配置更新
    emit gSigSent(Session::RequestString(2, GetModuleName(), "onConfigChanged", QJsonArray{ config_ }), session.socket);
    /*小车的界面信息定时更新*/
    static const int time_sync_interval = 30 * 1000; // 30秒
    static QTimer timer(gSerial);
    static auto SyncInfo = [this]() {
        /*此处应更新同步的主要的两个电池使用的是哪一个,还有一个是电量电压信息,因为下位机不会缓存指令,需要等待执行完一条指令后再执行下一条*/
        gSerial->WriteData(CAR_GET_MSG);
        //QThread::msleep(25); // 25ms 间隔不会有粘包的现象
        //gSerial->WriteData(CAR_GET_BATTERY_MSG);
    };
    static bool isInitConnect = false;
    if (!isInitConnect) {
        connect(&timer, &QTimer::timeout, this, SyncInfo);
        connect(&gTaskManager, &TaskManager::running, &timer, &QTimer::stop);
        connect(&gTaskManager, &TaskManager::waiting, [this]() {
            if (gSerial->isOpen()) {
                timer.start(time_sync_interval);
            }
            });
        isInitConnect = true;
    }
    SyncInfo();
}


void SerialPlugin::execute(const QString& method) {
    qDebug() << "[#Serial]执行方法" << method;
}

void SerialPlugin::scan(const Session& session) {
    gShare.on_session(session.ResponseSuccess(gSerial->GetAvailablePorts().join(",")), session.socket);
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
