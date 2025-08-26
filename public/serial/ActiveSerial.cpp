#include "ActiveSerial.h"

//定义上传数据处理函数
#pragma region FunctionCodeUploadDataHandler//上传数据处理函数
#ifdef DEVICE_TYPE_CAR
/*里程数据处理函数
* @param id 里程数据ID
* @param left 左里程数据
* @param right 右里程数据
*/
static void RecvMileageData(qint64 id,const MileageInfo& left,const MileageInfo& right) {
    static QString header = "ID\tLeftMileage\tLeftTime\tLeftTimeRaw\tRightMileage\tRightTime\tRightTimeRaw\n";
    static SavaDataFile mileage(QString("%1/mileage.txt").arg(kTaskDirCarName),header);
    //里程数据
    if (gTaskState == TaskState::TaskState_Running && mileage.initialize()) {
        //里程数据
        QString str = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n").arg(id)
            .arg((left.symbol ? -1 : 1) * left.pulse * speed_multiplier) //左里程数据
            .arg(gScannerCarTimeSync.GetScanner(left.time)) //推算的扫描仪时间
            .arg(left.time) //小车时间
            .arg((right.symbol ? -1 : 1) * right.pulse * speed_multiplier) //右里程数据
            .arg(gScannerCarTimeSync.GetScanner(right.time)) //推算的扫描仪时间
            .arg(right.time) //小车时间
            ;
        mileage.WriteLine(str);
        //里程数据 推送到订阅的客户端
        if (id % kMileageUpdateInterval == 0) {
            QJsonObject obj;
            obj.insert("id", id);
            obj.insert("left_mileage_symbol", left.symbol);
            obj.insert("left_mileage_pulse", left.pulse);
            obj.insert("left_mileage_time", left.time);
            obj.insert("right_mileage_symbol", right.symbol);
            obj.insert("right_mileage_pulse", right.pulse);
            obj.insert("right_mileage_time", right.time);
            PushClients(SUBSCRIBE_METHOD(Mileage), obj, sModuleSerial);
        }
    }
}
/*倾角计数据
* @param data 倾角计数据
*/
static void RecvInclinometerData(InclinometerInfo data) {
    static QString header = "X\tY\tTime\n";
    static SavaDataFile inclinometer_file(QString("%1/Inclinometer.txt").arg(kTaskDirCarName), header);
    //倾角计数据
    if (gTaskState == TaskState_Running && inclinometer_file.initialize()) {
        QString str = QString("%1\t%2\t%3\n").arg(data.x).arg(data.y).arg(data.time);
        inclinometer_file.WriteLine(str);
    }
}
/*扫描仪时间数据
* gGetScannerTimeCount 用于获取扫描仪时间数据的次数<en
* gScannerCarTimeSync 用于存储扫描仪和小车的时间
*/
static void RecvScannerTimeData() {//扫描仪时间数据
    static QString header = "ID\tTrolleyTime\tScannerTime\n";
    static SavaDataFile scanner_time_file(QString("%1/scannerTime.txt").arg(kTaskDirCarName), header);
    //扫描仪时间数据
    if (gTaskState == TaskState_Running && scanner_time_file.initialize()) {
        QString str = QString("%1\t%2\t%3\n").arg(gGetScannerTimeCount)
            .arg(gScannerCarTimeSync.car)
            .arg(gScannerCarTimeSync.scanner);
        scanner_time_file.WriteLineAndFlush(str);
    }
}
/*单里程数据 兼容前单里程的数据,默认不使用单里程数据,双里程数据中右里程数据为0
* @param id 里程数据ID
* @param data 里程数据
*/
static void RecvSingleMileageData(qint64 id, const MileageInfo& data) {
    static QString header = "ID\tMileage\tTime\tTimeRaw\n";
    static SavaDataFile single_mileage_file(QString("%1/singleMileage.txt").arg(kTaskDirCarName), header);
    //单里程数据
    if (gTaskState == TaskState_Running && single_mileage_file.initialize()) {
        QString str = QString("%1\t%2\t%3\t%4\n").arg(id)
            .arg((data.symbol ? -1 : 1) * data.pulse * speed_multiplier) //里程数据
            .arg(gScannerCarTimeSync.GetScanner(data.time)) //推算的扫描仪时间
            .arg(data.time); //小车时间
        single_mileage_file.WriteLine(str);
        //单里程数据 推送到订阅的客户端
        if (id % kMileageUpdateInterval == 0) {
            QJsonObject obj;
            obj.insert("id", id);
            obj.insert("mileage_symbol", data.symbol);
            obj.insert("mileage_pulse", data.pulse);
            obj.insert("mileage_time", data.time);
            PushClients(SUBSCRIBE_METHOD(Mileage), obj, sModuleSerial);
        }
    }
}
#endif // DEVICE_TYPE_CAR

#ifdef DEVICE_TYPE_CAMERA
/*相机触发数据
* @param data 相机触发数据
*/
static void RecvCameraPositionData(const CloverTriggerInfo& data) {
    static QString header = "ID\tCamID\tCentralPosition\ttime\tActualPosition\n";
    static SavaDataFile camera_position_file(QString("%1/centralPosition.txt").arg(kTaskDirCameraName), header);
    //相机触发数据
    if (gTaskState == TaskState_Running && camera_position_file.initialize()) {
        QString str = QString("%1\t%2\t%3\t%4\t%5\n").arg(data.id)
            .arg(data.camera_id) //相机ID
            .arg(data.position) //相机机位
            .arg(data.time) //小车时间
            .arg(data.GetActualPosition()); //实际机位
        camera_position_file.WriteLine(str);
        //相机触发数据 推送到订阅的客户端
        QJsonObject obj;
        obj.insert("id", data.id);
        obj.insert("camera_id", data.camera_id);
        obj.insert("central_position", data.position);
        obj.insert("time", data.time);
        obj.insert("actual_position", data.GetActualPosition());
        obj.insert("feedback", data.feedback);
        PushClients(SUBSCRIBE_METHOD(CameraPosition), obj,sModuleSerial);
        if (data.feedback == 0) {
            LOG_ERROR(QObject::tr("Camera trigger id:%1 camera:%2 position:%3 time:%4 feedback:%5 error").arg(data.id).arg(data.camera_id).arg(data.position).arg(data.time).arg(data.feedback));
        }
    }
}
#endif // DEVICE_TYPE_CAMERA

#pragma endregion

ActiveSerial::~ActiveSerial() {

}

Result ActiveSerial::SetConfig(const QJsonObject& config) {
    qDebug() << "[#Trolley]设置配置" << config;
    return Result();
}

void ActiveSerial::start(const Session& session) {
    //先启动相机,再启动小车 301项目是两个设备同时都有
#ifdef DEVICE_TYPE_CAMERA
    WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x01)));//相机启动 指令写入
#ifdef DEVICE_TYPE_CAR
    g_serial_session.addSession(CAMERA_START_STOP, [&session,this](const QJsonValue& result, const QString& message) {
        if (result.toInt(-1) == RESULT_SUCCESS) {//返回成功
            WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01))); //00：启动小车 01：启动并清除里程
            g_serial_session.addSession(CAR_STARTUP, session);
        } else {
            PushSessionResponse(session, result, message);
        }
        });
#else
    g_serial_session.addSession(CAMERA_START_STOP, session);
#endif // DEVICE_TYPE_CAR
#elif defined(DEVICE_TYPE_CAR)
    WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01)));//00：启动小车 01：启动并清除里程
    g_serial_session.addSession(CAR_STARTUP, session);
#else
    gShare.on_send(Result::Failure(tr("The device type is not defined")), session);
#endif // DEVICE_TYPE_CAR
}

void ActiveSerial::stop(const Session& session) {
    //先停止小车,再停止相机
#ifdef DEVICE_TYPE_CAR
    WriteData(CAR_STOP, QByteArray(1, static_cast<char>(0x01))); //00：停止小车 01：停止并清除里程
#ifdef DEVICE_TYPE_CAMERA
    g_serial_session.addSession(CAR_STOP, [=](const QJsonValue& result, const QString& message) {
        if (result.toInt(-1) == RESULT_SUCCESS) {//返回成功
            WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x00)));
            g_serial_session.addSession(CAMERA_START_STOP, session);
        } else {
            PushSessionResponse(session, result, message);
        }
        });
#else
    g_serial_session.addSession(CAR_STOP, session);
#endif // DEVICE_TYPE_CAMERA
#elif defined(DEVICE_TYPE_CAMERA)
    WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x00)));
    g_serial_session.addSession(CAMERA_START_STOP, session);
#else
    gShare.on_send(Result::Failure(tr("The device type is not defined")), session);
#endif // DEVICE_TYPE_CAR
}

Result ActiveSerial::OnStarted(CallbackResult callback) {
    //先启动相机,再启动小车 301项目是两个设备同时都有
#ifdef DEVICE_TYPE_CAMERA
    WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x01)));//相机启动 指令写入
#ifdef DEVICE_TYPE_CAR
    g_serial_session.addSession(CAMERA_START_STOP, [=](const QJsonValue& result, const QString&) {
        if (result.toInt(-1) == RESULT_SUCCESS) {//返回成功
            WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01))); //00：启动小车 01：启动并清除里程
            g_serial_session.addSession(CAR_STARTUP, [callback](const QJsonValue& result, const QString&) {
                callback(result.toInt(-1) == RESULT_SUCCESS ? true : false);
                });
        } else {
            callback(false);
        }
        });
#else
    g_serial_session.addSession(CAMERA_START_STOP, [callback](const QJsonValue& result, const QString&) {
        callback(result.toInt(-1) == RESULT_SUCCESS ? true : false);
        });
#endif // DEVICE_TYPE_CAR
#elif defined(DEVICE_TYPE_CAR)
    WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01)));//00：启动小车 01：启动并清除里程
    g_serial_session.addSession(CAR_STARTUP, [callback](const QJsonValue& result, const QString&) {
        callback(result.toInt(-1) == RESULT_SUCCESS ? true : false);
        });
#else
    LOG_ERROR(tr("The device type is not defined"));
    callback(false);
#endif // DEVICE_TYPE_CAR
    return Result();
}

Result ActiveSerial::OnStopped(CallbackResult callback) {

    //先停止小车,再停止相机
#ifdef DEVICE_TYPE_CAR
    WriteData(CAR_STOP, QByteArray(1, static_cast<char>(0x01))); //00：停止小车 01：停止并清除里程
#ifdef DEVICE_TYPE_CAMERA
    g_serial_session.addSession(CAR_STOP, [=](const QJsonValue& result, const QString&) {
        if (result.toInt(-1) == RESULT_SUCCESS) {//返回成功
            WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x00)));
            g_serial_session.addSession(CAMERA_START_STOP, [callback](const QJsonValue& result, const QString&) {
                callback(result.toInt(-1) == RESULT_SUCCESS ? true : false);
                });
        } else {
            callback(false);
        }
        });
#else
    g_serial_session.addSession(CAR_STOP, [callback](const QJsonValue& result, const QString&) {
        callback(result.toInt(-1) == RESULT_SUCCESS ? true : false);
        });
#endif // DEVICE_TYPE_CAMERA
#elif defined(DEVICE_TYPE_CAMERA)
    WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x00)));
    g_serial_session.addSession(CAMERA_START_STOP, [callback](const QJsonValue& result, const QString&) {
        callback(result.toInt(-1) == RESULT_SUCCESS ? true : false);
        });
#else
    LOG_ERROR(tr("The device type is not defined"));
    callback(false);
#endif // DEVICE_TYPE_CAR
    return Result();
}

bool ActiveSerial::HandleProtocol(FunctionCodeType code, const QByteArray& data) {
    qDebug() << "Received Function Code:" << QString::number(code, 16).rightJustified(2, '0').toUpper() << "Data:" << data.toHex().toUpper();
    //if (data.isEmpty()) return;
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);  // 设置字节序
    QJsonValue result; QString message;

#pragma region FunctionCode
    /*第一部分:命令执行的回复,需要响应请求,break后统一处理
    * 第二部分:获取数据的回复,需要解析数据,然后break响应请求,或者return直接返回
    * 第三部分:自动上传的数据格式,直接return不回复
    */
    switch (code) {
#ifdef DEVICE_TYPE_CAR
    case SCANER_SET_AUTOMATION_TIME:
        //[[fallthrough]];  // C++17特性，明确表示故意fall through
    case CAR_SCANER_PWR_CTRL:
    case CAR_SET_ENCODER_USAGE:
    case CAR_SET_START_STOP_BUTTON_MODE:
    case CAR_STARTUP:
    case CAR_STOP:
    case CAR_CHANGING_OVER:
    case CAR_SET_SPEED:
    case CAR_RESET_TOTAL_MILEAGE:
#endif // DEVICE_TYPE_CAR
#ifdef DEVICE_TYPE_CAMERA
    case CAMERA_START_STOP:
    case CAMERA_SET_ENABLE_MODE:
    case CAMERA_LED_CONTROL:
    case CAMERA_SET_EXPOSURE_TIME://00：成功 01：失败，时间范围不正确（编码器 2 位置） 02：时间失败，时间范围不正确（编码器 1 位置） 03：失败，时间与灯控 板返回时间不一致（编码器 2 位 置） 04：失败，时间与灯控 板返回时间不一致（编码器 1 位 置）05：未检测到灯控板或灯控板无 响应
    case CAMERA_MOTOR_SPEED_CONTROL:
    case CAMERA_MOTOR_START_STOP_CONTROL://00：正常 01：已经在旋转 02：超时
    case CAMERA_SET_MOTOR_POSITION:
    case CAMERA_SET_TRIGGER_POSITION:
#endif // DEVICE_TYPE_CAMERA
        stream >> u8result;
        result = u8result;
        break;
#ifdef DEVICE_TYPE_CAR
    case SCANER_GET_AUTOMATION_TIME:
    {
        quint64 autotime;
        stream >> autotime >> gScannerCarTimeSync.car;
        //获取扫描仪时间失败,返回0 应该执行任务回调
        if (autotime == 0) {
            LOG_ERROR(tr("Failed to get scanner time"));
            result = false;
            return false;
        }
        gScannerCarTimeSync.scanner = autotime;
        RecvScannerTimeData();//扫描仪时间数据的任务处理
        result = true;
    }return true;
    case CAR_GET_MSG:
    {
        static CarInfo info_;
        stream >> info_.speed >> info_.temperature_symbol >> info_.temperature << info_.direction << info_.moving << info_.battery_usage << info_.knob_state << info_.encoder_mode << info_.key_mode << info_.scanner_power << info_.speed_min << info_.speed_max << info_.time;
        QJsonObject obj;
        obj.insert("speed", info_.speed);
        obj.insert("temperature", info_.temperature_symbol ? -1 * info_.temperature / 10.0 : info_.temperature / 10.0);
        obj.insert("direction", info_.direction);
        obj.insert("moving", info_.moving);
        obj.insert("battery_usage", info_.battery_usage);
        obj.insert("knob_state", info_.knob_state);
        obj.insert("encoder_mode", info_.encoder_mode);
        obj.insert("key_mode", info_.key_mode);
        obj.insert("scanner_power", info_.scanner_power);
        obj.insert("speed_min", info_.speed_min);
        obj.insert("speed_max", info_.speed_max);
        obj.insert("time", info_.time);
        result = obj;
    }break;
    case CAR_GET_TOTAL_MILEAGE:
    {
        static qint64 total_mileage{ 0 }; //小车总里程 单位为米
        stream >> total_mileage;
        result = total_mileage;
    }break;
    case CAR_GET_BATTERY_MSG:
    {
        static BatteryInfo left_battery_info_;
        static BatteryInfo right_battery_info_;
        stream >> left_battery_info_.state >> left_battery_info_.voltage >> left_battery_info_.current << left_battery_info_.remainingCapacity >> left_battery_info_.fullChargeCapacity << left_battery_info_.cycleCount;
        stream >> right_battery_info_.state >> right_battery_info_.voltage >> right_battery_info_.current << right_battery_info_.remainingCapacity >> right_battery_info_.fullChargeCapacity << right_battery_info_.cycleCount;
        QJsonObject obj;
        obj.insert("left_battery_state", left_battery_info_.state);
        obj.insert("left_battery_voltage", left_battery_info_.voltage);
        obj.insert("left_battery_current", left_battery_info_.current);
        obj.insert("left_battery_remainingCapacity", left_battery_info_.remainingCapacity);
        obj.insert("left_battery_fullChargeCapacity", left_battery_info_.fullChargeCapacity);
        obj.insert("left_battery_cycleCount", left_battery_info_.cycleCount);
        obj.insert("right_battery_state", right_battery_info_.state);
        obj.insert("right_battery_voltage", right_battery_info_.voltage);
        obj.insert("right_battery_current", right_battery_info_.current);
        obj.insert("right_battery_remainingCapacity", right_battery_info_.remainingCapacity);
        obj.insert("right_battery_fullChargeCapacity", right_battery_info_.fullChargeCapacity);
        obj.insert("right_battery_cycleCount", right_battery_info_.cycleCount);
        result = obj;
    }break;

    /**自动上传的数据格式,直接return不回复**/
    case 0xEF://倾角信息上传 倾角计的量程为正负 15 度。
    {
        InclinometerInfo inclinometer;
        stream >> inclinometer.x >> inclinometer.y >> inclinometer.time;
        RecvInclinometerData(inclinometer);//倾角计数据的任务处理
    }return true;
    case 0xF8://默认只支持双里程数据
    {
        static MileageInfo  left_mileage{0};
        static MileageInfo  right_mileage{0};
        stream >> left_mileage.symbol >> left_mileage.pulse >> left_mileage.time;
        stream >> right_mileage.symbol >> right_mileage.pulse >> right_mileage.time;
        RecvMileageData(mileage_count_, left_mileage, right_mileage);//里程数据的任务处理

        mileage_count_++;
    }return true;
    case 0xFC://按键信息上传 保留处理
    {
        LOG_INFO(QString("Received Car key Data:%1").arg(data.toHex().toUpper()));
        quint8 key_value, knob_value;
        stream >> key_value >> knob_value;
        switch (key_value) {//启动和停止按键不会目前不会上传
        case 0x02://启动按键 （小车优先级高）
        case 0x03://停止按键 （小车优先级高）
        case 0x04://开始扫描按键 （软件优先级高）
            LOG_INFO("开始扫描按键");
            break;
        case 0x05://停止扫描按键 （软件优先级高）
            LOG_INFO("停止扫描按键");
            break;
        case 0x0A://方向切换按钮
        case 0x0B://模式切换按钮（长按启停按钮）
        default:
            break;
        }
        PushClients(SUBSCRIBE_METHOD(CarKey), key_value, sModuleSerial);
    }return true;
    case 0xFD://扫描仪CAN指令上传 待测试
    {
        LOG_INFO(QString("Received Scanner CAN Data:%1").arg(data.toHex().toUpper()));
        quint8 can_id, trigger_in, trigger_out, ack_by_can, ditection;
        quint64 can_data;
        stream >> can_id >> trigger_in >> trigger_out >> ack_by_can >> ditection >> can_data;
        switch (can_id) {
        case 0x01://End Scan Operation
            LOG_INFO(tr("End Scan Operation"));
            break;
        case 0x02://Initiate Scan Operation
            //Out: Started scan operation mode 0X->Spherical Scan 1X->Helical TTL Scan 2X->Helical CAN Scan
            LOG_INFO(tr("Initiate Scan Operation"));
            break;
        case 0x03://Set Automation Time
        case 0x04:
        case 0x05://Record/Pause
            //In / Out : 0X->Pause scan recording 1X->Scan recording
        case 0x07://Data Message
        case 0x08:
        case 0x09://Self-Test
        case 0x0A://Get Automation Time
        case 0x0B://Mirror Index Trigger Enable
            //0X->Trigger on mirror index disabled 1X->Trigger onmirror index enabled
        case 0x0C://Mirror Index Trigger Occurred镜像索引触发器已发生
            //Out: Automation time of occurred mirror index
        default:
            break;
        }
        //QJsonObject obj;
        //obj.insert("flag", flag);
        //obj.insert("data", data);
        //result = obj;
    }return true;
    case 0xFE://为了兼容单里程数据
    {
        MileageInfo mileage{};
        stream >> mileage.symbol >> mileage.pulse >> mileage.time;
        RecvSingleMileageData(mileage_count_, mileage);
        mileage_count_++;
    }return true;
#endif // DEVICE_TYPE_CAR
#ifdef DEVICE_TYPE_CAMERA
    case CAMERA_GET_MSG:
    {
        stream >> clover_info_.enable >> clover_info_.mode;
        stream.readRawData(reinterpret_cast<char*>(clover_info_.fps), 10);
        QJsonObject obj;
        obj.insert("enable", clover_info_.enable);
        obj.insert("mode", clover_info_.mode);
        obj.insert("fps", clover_info_.fps[0]);
        //统一,不每个使用
        //QJsonArray jsonArray;
        //for (const auto& fps : clover_info_.fps) {
        //    jsonArray.append(fps);
        //}
        result = obj;
    }break;
    case CAMERA_GET_LED_FAN_STATUS:
    {
        stream >> clover_info_.led_state >> clover_info_.fan_state;
        QJsonObject obj;
        obj.insert("led_state", clover_info_.led_state);
        obj.insert("fan_state", clover_info_.fan_state);
        result = obj;
    }break;
    case  CAMERA_READ_MOTOR_STATUS:
    {
        stream >> clover_info_.speed >> clover_info_.direction >> clover_info_.moving >> clover_info_.calibrated >> clover_info_.pose;
        QJsonObject obj;
        obj.insert("speed", clover_info_.speed);
        obj.insert("direction", clover_info_.direction);
        obj.insert("moving", clover_info_.moving);
        obj.insert("calibrated", clover_info_.calibrated);
        obj.insert("pose", clover_info_.pose);
        result = obj;
    }break;
    case  CAMERA_GET_TRIGGER_POSITION:
    {
        stream >> clover_info_.trigger_position[0] >> clover_info_.trigger_position[1] >> clover_info_.trigger_position[2];
        QJsonObject obj;
        obj.insert("trigger_position_0", clover_info_.trigger_position[0]);
        obj.insert("trigger_position_1", clover_info_.trigger_position[1]);
        obj.insert("trigger_position_2", clover_info_.trigger_position[2]);
        result = obj;
    }break;
    /**自动上传的数据格式**/
    case 0xF5://相机时间数据上传
    {
        CloverTriggerInfo trigger{};
        stream >> trigger.id >> trigger.time >> trigger.camera_id >> trigger.feedback >> trigger.position;
        RecvCameraPositionData(trigger);//相机位置数据的任务处理

    }return true;
#endif // DEVICE_TYPE_CAMERA
    //功能码,获取设备类型
    case CODE_TEST:
    {
        quint8 type;
        stream >> type;
        result = type;//由界面设置设备类型
        if ((kSupportedSerialDevices & type) == 0) {
            LOG_ERROR(tr("The device type that is currently set is inconsistent with the device type that is currently supported!"));
        }
        //kSupportedSerialDevices = type;
        gShare.RegisterSettings->setValue("type", type);//注册表类型设置
    }break;
    default:
        return SerialPortTemplate::HandleProtocol(code, data);
    }
#pragma endregion
    //如何对应到具体的请求?
    SerialSession::instance().HandleTrolleySession(code, result, message);
    return true;
};
