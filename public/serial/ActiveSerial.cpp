#include "ActiveSerial.h"

//定义上传数据处理函数
#pragma region FunctionCodeUploadDataHandler//上传数据处理函数
#ifdef DEVICE_TYPE_CAR
#include "public/utils/car_mileage_correction.h"
/*里程数据处理函数
* @param id 里程数据ID
* @param left 左里程数据
* @param right 右里程数据
*/
inline static void RecvMileageData(qint64 id, const MileageInfo& left, const MileageInfo& right) {
    static QString header = "ID\tLeftMileage\tLeftTime\tLeftTimeRaw\tRightMileage\tRightTime\tRightTimeRaw\n";
    static SavaDataFile mileage(QString("%1/mileage.txt").arg(kTaskDirCarName), header);
    if (mileage.create_file()) { 
        //里程数据
        QString str = QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\n").arg(id)
            .arg((left.symbol ? -1 : 1) * left.pulse * g_mileage_multiplier) //左里程数据
            .arg(gScannerCarTimeSync.GetScanner(left.time)) //推算的扫描仪时间
            .arg(left.time) //小车时间
            .arg((right.symbol ? -1 : 1) * right.pulse * g_mileage_multiplier) //右里程数据
            .arg(gScannerCarTimeSync.GetScanner(right.time)) //推算的扫描仪时间
            .arg(right.time) //小车时间
            ;
        mileage.WriteLine(str);
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
    if (single_mileage_file.create_file()) {
        QString str = QString("%1\t%2\t%3\t%4\n").arg(id)
            .arg((data.symbol ? -1 : 1) * data.pulse) //里程数据
            .arg(gScannerCarTimeSync.GetScanner(data.time)) //推算的扫描仪时间
            .arg(data.time); //小车时间
        single_mileage_file.WriteLine(str);
    }
}
/*倾角计数据
* @param data 倾角计数据
*/
inline static void RecvInclinometerData(InclinometerInfo data) {
    static QString header = "X\tY\tTime\n";
    static SavaDataFile inclinometer_file(QString("%1/Inclinometer.txt").arg(kTaskDirCarName), header);
    if (inclinometer_file.create_file()) {
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
    if (scanner_time_file.create_file()) {
        QString str = QString("%1\t%2\t%3\n").arg(gGetScannerTimeCount)
            .arg(gScannerCarTimeSync.car)
            .arg(gScannerCarTimeSync.scanner);
        scanner_time_file.WriteLineAndFlush(str);
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
    if (gTaskState == TaskState_Running && camera_position_file.create_file()) {
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
        PushClients(SUBSCRIBE_METHOD(CameraPosition), obj, sModuleSerial);
        if (data.feedback == 0) {
            LOG_ERROR(QObject::tr("Camera trigger id:%1 camera:%2 position:%3 time:%4 feedback:%5 error").arg(data.id).arg(data.camera_id).arg(data.position).arg(data.time).arg(data.feedback));
        }
    }
}
#endif // DEVICE_TYPE_CAMERA

#pragma endregion

ActiveSerial::~ActiveSerial() {
    qDebug() << "模块~ActiveSerial 退出" << QThread::currentThread();
}

Result ActiveSerial::SetConfig(const QJsonObject& config) {
    qDebug() << "[#Trolley]设置配置" << config;
    return Result();
}

void ActiveSerial::start(const Session& session) {
    //先启动相机,再启动小车 301项目是两个设备同时都有 201是分开的,指令也不同
#ifdef DEVICE_TYPE_CAMERA
    WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x01)));//相机启动 指令写入
    g_serial_session.addSession(CAMERA_START_STOP, session);
#endif // DEVICE_TYPE_CAR

#ifdef DEVICE_TYPE_CAR
#ifdef DEVICE_TYPE_LINE_SACN_CAMERA
    if (gShare.GetHandlerList().contains(sModuleCamera)) {
        WriteData(LINE_SACN_CAMERA_CONTROL, QByteArray(1, static_cast<char>(0x01)));//相机启动 指令写入
        g_serial_session.addSession(LINE_SACN_CAMERA_CONTROL, [&session, this](const qint8& i8result, const QJsonValue& value) {
            if (i8result == RESULT_SUCCESS) {
                WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01)));//00：启动小车 01：启动并清除里程
                g_serial_session.addSession(CAR_STARTUP, session);
            } else {
                emit gSigSent(session.Finished(i8result, value.toString()), session.socket);
            }
            });
    } else {
        WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01)));//00：启动小车 01：启动并清除里程
        g_serial_session.addSession(CAR_STARTUP, session);
    }
#else // DEVICE_TYPE_LINE_SACN_CAMERA
    WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01)));//00：启动小车 01：启动并清除里程
    g_serial_session.addSession(CAR_STARTUP, session);
#endif
#endif // DEVICE_TYPE_CAR
}

void ActiveSerial::stop(const Session& session) {
    //先停止小车,再停止相机
#ifdef DEVICE_TYPE_CAR
    WriteData(CAR_STOP, QByteArray(1, static_cast<char>(0x01))); //00：停止小车 01：停止并清除里程
#ifdef DEVICE_TYPE_LINE_SACN_CAMERA
    if (gShare.GetHandlerList().contains(sModuleCamera)) {
        g_serial_session.addSession(CAR_STOP, [&session, this](const qint8& i8result, const QJsonValue& value) {
            if (i8result == RESULT_SUCCESS) {
                WriteData(LINE_SACN_CAMERA_CONTROL, QByteArray(1, static_cast<char>(0x00)));
                g_serial_session.addSession(LINE_SACN_CAMERA_CONTROL, session);
            } else {
                emit gSigSent(session.Finished(i8result, value.toString()), session.socket);
            }
            });
    } else {
        g_serial_session.addSession(CAR_STOP, session);
    }
#else // DEVICE_TYPE_LINE_SACN_CAMERA
    g_serial_session.addSession(CAR_STOP, session);
#endif
#endif // DEVICE_TYPE_CAMERA

#ifdef DEVICE_TYPE_CAMERA
    WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x00)));
    g_serial_session.addSession(CAMERA_START_STOP, session);
#endif // DEVICE_TYPE_CAR
}

Result ActiveSerial::OnStarted(const CallbackResult& callback) {
    //先启动相机,再启动小车 301项目是两个设备同时都有
#ifdef DEVICE_TYPE_CAMERA
    WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x01)));//相机启动 指令写入
    g_serial_session.addSession(CAMERA_START_STOP, callback);
#endif // DEVICE_TYPE_CAR

#ifdef DEVICE_TYPE_CAR
#ifdef DEVICE_TYPE_LINE_SACN_CAMERA
    if (gShare.GetHandlerList().contains(sModuleCamera)) {
        WriteData(LINE_SACN_CAMERA_CONTROL, QByteArray(1, static_cast<char>(0x01)));//相机启动 指令写入
        g_serial_session.addSession(LINE_SACN_CAMERA_CONTROL, [callback, this](const qint8& i8result, const QJsonValue& value) {
            if (i8result == RESULT_SUCCESS) {
                WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01)));//00：启动小车 01：启动并清除里程
                g_serial_session.addSession(CAR_STARTUP, callback);
            } else {
                return callback(i8result, value);
            }
            });
    } else {
        WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01)));//00：启动小车 01：启动并清除里程
        g_serial_session.addSession(CAR_STARTUP, callback);
    }
#else // DEVICE_TYPE_LINE_SACN_CAMERA
    WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01)));//00：启动小车 01：启动并清除里程
    g_serial_session.addSession(CAR_STARTUP, callback);
#endif
#endif // DEVICE_TYPE_CAR
    return Result();
}

Result ActiveSerial::OnStopped(const CallbackResult& callback) {
    //先停止小车,再停止相机
#ifdef DEVICE_TYPE_CAR
    WriteData(CAR_STOP, QByteArray(1, static_cast<char>(0x01))); //00：停止小车 01：停止并清除里程
#ifdef DEVICE_TYPE_LINE_SACN_CAMERA
    if (gShare.GetHandlerList().contains(sModuleCamera)) {
        g_serial_session.addSession(CAR_STOP, [callback, this](const qint8& i8result, const QJsonValue& value) {
            if (i8result == RESULT_SUCCESS) {
                WriteData(LINE_SACN_CAMERA_CONTROL, QByteArray(1, static_cast<char>(0x00)));
                g_serial_session.addSession(LINE_SACN_CAMERA_CONTROL, callback);
            } else if (callback) {
                return callback(i8result, value);
            }
            });
    } else {
        g_serial_session.addSession(CAR_STOP, callback);
    }
#else // DEVICE_TYPE_LINE_SACN_CAMERA
    g_serial_session.addSession(CAR_STOP, callback);
#endif
#endif // DEVICE_TYPE_CAMERA

#ifdef DEVICE_TYPE_CAMERA
    WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x00)));
    g_serial_session.addSession(CAMERA_START_STOP, callback);
#endif // DEVICE_TYPE_CAR
    return Result();
}

bool ActiveSerial::HandleProtocol(FunctionCodeType code, const QByteArray& data) {
    qDebug() << "#Received Code:" << QString::number(code, 16).rightJustified(2, '0').toUpper() << "Data:" << data.toHex().toUpper();
    //if (data.isEmpty()) return;
    static quint8 invoke_trolley = share::ModuleName::trolley;
    static auto push_car = [&code,&data]() -> void {
        QByteArray bytes = QByteArray(1, invoke_trolley) + QByteArray(1, code) + data;
        //bytes.append(invoke_trolley).append(code).append(data);
        qDebug() << "bytes:" << bytes.toHex().toUpper();
        emit gShare.sigSentBinary(bytes);//推送二进制数据
    };
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);  // 设置字节序
    QJsonValue value;

#pragma region FunctionCode
    /*第一部分:命令执行的回复,需要响应请求,break后统一处理
    * 第二部分:获取数据的回复,需要解析数据,然后 break响应请求,或者 return 直接返回
    * 第三部分:自动上传的数据格式,直接 return 不回复
    */
    switch (code) {
#ifdef DEVICE_TYPE_CAR
    case AUTOMATION_TIME_SYNC:
        stream >> i8result;
        if (i8result == RESULT_SUCCESS) {
            ScannerAndCarTimeInfo::isAwake = true;
        }
        break;
    case CAR_STARTUP:
    case CAR_STOP:
        //[[fallthrough]];  // C++17特性，明确表示故意 fall through
    case CAR_CHANGING_OVER:
    case CAR_SET_SPEED:
    case CAR_RESET_TOTAL_MILEAGE:
    case CAR_SCANER_PWR_CTRL:
    case CAR_SET_ENCODER_USAGE:
    case CAR_SET_START_STOP_BUTTON_MODE:
    case CAR_SWITCH_VOLTAGE_CTRL:
    case CAR_CHOOSE_BATTERY_SOURCE:
    case CAR_SET_SCANNER_HEIGHT:
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
        stream >> i8result;
        break;
#ifdef DEVICE_TYPE_CAR
    case SCANER_GET_AUTOMATION_TIME:
    {
        gGetScannerTimeCount++;
        quint64 autotimer;
        quint64 car_time;
        stream >> autotimer >> car_time;
        //获取扫描仪时间失败,返回0 应该执行任务回调
        if (autotimer == 0) {
            LOG_ERROR(tr("获取扫描仪时间同步失败,返回 扫描仪值:%1 小车值:%2,原始数据").arg(autotimer).arg(gScannerCarTimeSync.car).arg(data.toHex().toUpper()));
            i8result = -1;
            break;
        }
        gScannerCarTimeSync.update(autotimer,car_time);
        if (gTaskState == TaskState::TaskState_Running) {
            RecvScannerTimeData();//扫描仪时间数据的任务处理
        }
        i8result = 0;
    }break;
    case CAR_GET_MSG:
    case CAR_GET_INFO:
    {
        push_car();
        //获取小车信息后,还有获取电池信息等
        WriteData(CAR_GET_BATTERY_MSG);
    }return true;
    case CAR_GET_TOTAL_MILEAGE:
    {
        static qint64 total_mileage{ 0 }; //小车总里程 单位为米
        stream >> total_mileage;
        value = total_mileage;
        i8result = 0;
    }break;
    //直接推送消息到小车的解析数据
    case CAR_GET_BATTERY_MSG:
    {
        push_car();
    }return true;
    /**自动上传的数据格式,return 不回复**/
    case 0xEF://倾角信息上传 倾角计的量程为正负 15 度。
    {
        if (gTaskState == TaskState::TaskState_Running) {
            InclinometerInfo inclinometer;
            stream >> inclinometer;
            RecvInclinometerData(inclinometer);//倾角计数据的任务处理
        }
    }return true;
    case 0xF8://默认只支持双里程数据
    {
        static MileageInfo left;
        static MileageInfo right;
        stream >> left >> right;
        if (left.symbol != right.symbol) {
            LOG_WARNING(tr("[#串口]左右里程数据符号不一致 左: %1 右: %2").arg(left.symbol,0,16).arg(right.symbol,0,16));
            qWarning() << left.symbol << left.pulse << left.time;
            qWarning() << right.symbol << right.pulse << right.time;
            //return true;//这个时候抛弃掉数据就好了
        }
        left.time *= 10; right.time *= 10;//时间单位为10us,需要*10改为us的单位
        struMileage mileage = MileageCorrector::instance().Correct(left.time, left.pulse, right.time, right.pulse);
        //里程数据 推送到订阅的客户端
        if (g_mileage_count % kMileageUpdateInterval == 0) {
            //二进制数据 发送
            QByteArray bytes = Serialize::Bytes(invoke_trolley, code, g_mileage_count, left.symbol, mileage.pulse * g_mileage_multiplier, mileage.time);
            emit gShare.sigSentBinary(bytes);//推送二进制数据
        }
        if (gTaskState == TaskState::TaskState_Running) {
            qDebug() << "里程数据的任务处理:" << g_mileage_count << "mileage:" << mileage.pulse << "time:" << mileage.time;
            MileageInfo  mileage_info(right.symbol, mileage.pulse, mileage.time);
            RecvSingleMileageData(g_mileage_count, mileage_info);
            RecvMileageData(g_mileage_count, left, right);//里程数据的任务处理
        }
        g_mileage_count++;
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
    case 0xFD://扫描仪CAN指令上传
    {
        LOG_INFO(QString("扫描仪CAN指令上传:%1").arg(data.toHex().toUpper()));
        quint8 can_id, trigger_in, trigger_out, ack_by_can, detection;
        quint64 can_data;
        stream >> can_id >> trigger_in >> trigger_out >> ack_by_can >> detection >> can_data;
        switch (can_id) {
        case 0x01://End Scan Operation
            LOG_INFO(tr("CAN指令上传:扫描仪停止"));
            break;
        case 0x02://Initiate Scan Operation
            //Out: Started scan operation mode 0X->Spherical Scan 1X->Helical TTL Scan 2X->Helical CAN Scan
            LOG_INFO(tr("CAN指令上传:扫描仪启动"));
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
            //0X->Trigger on mirror index disabled 1X->Trigger on mirror index enabled
        case 0x0C://Mirror Index Trigger Occurred镜像索引触发器已发生
            //Out: Automation time of occurred mirror index
        default:
            break;
        }
    }return true;
    case 0xFE://为了兼容单里程数据
    {
        MileageInfo mileage{};
        stream >> mileage.symbol >> mileage.pulse >> mileage.time;
        g_mileage_count++;
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
        value = obj;
    }break;
    case CAMERA_GET_LED_FAN_STATUS:
    {
        stream >> clover_info_.led_state >> clover_info_.fan_state;
        QJsonObject obj;
        obj.insert("led_state", clover_info_.led_state);
        obj.insert("fan_state", clover_info_.fan_state);
        value = obj;
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
        value = obj;
    }break;
    case  CAMERA_GET_TRIGGER_POSITION:
    {
        stream >> clover_info_.trigger_position[0] >> clover_info_.trigger_position[1] >> clover_info_.trigger_position[2];
        QJsonObject obj;
        obj.insert("trigger_position_0", clover_info_.trigger_position[0]);
        obj.insert("trigger_position_1", clover_info_.trigger_position[1]);
        obj.insert("trigger_position_2", clover_info_.trigger_position[2]);
        value = obj;
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
        value = type;//由界面设置设备类型
        qDebug() << "当前设备类型:" << type; // 1 中控 2三叶草相机 3 三叶草21机位
        //if ((kSupportedSerialDevices & type) == 0) {
            //LOG_ERROR(tr("The device type that is currently set is inconsistent with the device type that is currently supported!"));
        //}
        if (uCurrentSerialDevice != type) {
            uCurrentSerialDevice = type;
            gShare.RegisterSettings->setValue("type", type);//注册表类型设置
        }
    }break;
    default:
        return SerialPortTemplate::HandleProtocol(code, data);
    }
#pragma endregion
    //如何对应到具体的请求?
    SerialSession::instance().HandleSessionCallback(code, i8result, value);
    return true;
};
