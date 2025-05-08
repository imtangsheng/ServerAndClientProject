#include "ActiveSerial.h"


ActiveSerial::~ActiveSerial() {

}


Result ActiveSerial::SetConfig(const QJsonObject& config) {
    qDebug() << "[#Trolley]��������" << config;
    return Result();
}

Result ActiveSerial::start() {
    Result result(false);
    //���������,������С��
#ifdef DEVICE_TYPE_CLOVER
    result = WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x01)));
#endif // DEVICE_TYPE_CLOVER
#ifdef DEVICE_TYPE_CAR
    result = WriteData(CAR_STARTUP, QByteArray(1, static_cast<char>(0x01))); //00������С�� 01��������������
#endif // DEVICE_TYPE_CAR

    return result;
}

Result ActiveSerial::stop() {
    Result result(false);
    //��ֹͣС��,��ֹͣ���
#ifdef DEVICE_TYPE_CAR
    result = WriteData(CAR_STOP, QByteArray(1, static_cast<char>(0x01))); //00��ֹͣС�� 01��ֹͣ��������
#endif // DEVICE_TYPE_CAR
#ifdef DEVICE_TYPE_CLOVER
    result = WriteData(CAMERA_START_STOP, QByteArray(1, static_cast<char>(0x00)));
#endif // DEVICE_TYPE_CLOVER
    return result;
}

bool ActiveSerial::HandleProtocol(FunctionCodeType code, const QByteArray& data) {
    qDebug() << "Received Function Code:" << QString::number(code, 16).rightJustified(2, '0').toUpper() << "Data:" << data.toHex().toUpper();
    //if (data.isEmpty()) return;
    QDataStream stream(data);
    stream.setByteOrder(QDataStream::LittleEndian);  // �����ֽ���
    QJsonValue result; QString message;

#pragma region FunctionCode
    /*��һ����:����ִ�еĻظ�,��Ҫ��Ӧ����,break��ͳһ����
    * �ڶ�����:��ȡ���ݵĻظ�,��Ҫ��������,Ȼ��break��Ӧ����,����returnֱ�ӷ���
    * ��������:�Զ��ϴ������ݸ�ʽ,ֱ��return���ظ�
    */
    switch (code) {
#ifdef DEVICE_TYPE_CAR
    case SCANER_SET_AUTOMATION_TIME:
        //[[fallthrough]];  // C++17���ԣ���ȷ��ʾ����fall through
    case CAR_SCANER_PWR_CTRL:
    case CAR_SET_ENCODER_USAGE:
    case CAR_SET_START_STOP_BUTTON_MODE:
    case CAR_STARTUP:
    case CAR_STOP:
    case CAR_CHANGING_OVER:
    case CAR_SET_SPEED:
    case CAR_RESET_TOTAL_MILEAGE:
#endif // DEVICE_TYPE_CAR
#ifdef DEVICE_TYPE_CLOVER
    case CAMERA_START_STOP:
    case CAMERA_SET_ENABLE_MODE:
    case CAMERA_LED_CONTROL:
    case CAMERA_SET_EXPOSURE_TIME://00���ɹ� 01��ʧ�ܣ�ʱ�䷶Χ����ȷ�������� 2 λ�ã� 02��ʱ��ʧ�ܣ�ʱ�䷶Χ����ȷ�������� 1 λ�ã� 03��ʧ�ܣ�ʱ����ƿ� �巵��ʱ�䲻һ�£������� 2 λ �ã� 04��ʧ�ܣ�ʱ����ƿ� �巵��ʱ�䲻һ�£������� 1 λ �ã�05��δ��⵽�ƿذ��ƿذ��� ��Ӧ
    case CAMERA_MOTOR_SPEED_CONTROL:
    case CAMERA_MOTOR_START_STOP_CONTROL://00������ 01���Ѿ�����ת 02����ʱ
    case CAMERA_SET_MOTOR_POSITION:
    case CAMERA_SET_TRIGGER_POSITION:
#endif // DEVICE_TYPE_CLOVER
        stream >> u8result;
        result = u8result;
        break;
#ifdef DEVICE_TYPE_CAR
    case SCANER_GET_AUTOMATION_TIME:
    {
        quint64 autotime;
        stream >> autotime >> scanner_time_info_.car;
        //��ȡɨ����ʱ��ʧ��,����0 Ӧ��ִ������ص�
        if (autotime == 0) {
            LOG_ERROR(tr("Failed to get scanner time"));
            result = false;
            return false;
        }
        scanner_time_info_.scanner = autotime;
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
        static qint64 total_mileage{ 0 }; //С������� ��λΪ��
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

    /**�Զ��ϴ������ݸ�ʽ,ֱ��return���ظ�**/
    case 0xEF://�����Ϣ�ϴ� ��ǼƵ�����Ϊ���� 15 �ȡ�
    {
        InclinometerInfo inclinometer;
        stream >> inclinometer.x >> inclinometer.y >> inclinometer.time;
    }return true;
    case 0xF8://Ĭ��ֻ֧��˫�������
    {
        static MileageInfo  left_mileage{0};
        static MileageInfo  right_mileage{0};
        stream >> left_mileage.symbol >> left_mileage.pulse >> left_mileage.time;
        stream >> right_mileage.symbol >> right_mileage.pulse >> right_mileage.time;
        if (mileage_count_ % kMileageUpdateInterval == 0) {
            QJsonObject obj;
            obj.insert("mileage_count", mileage_count_);
            obj.insert("left_mileage_symbol", left_mileage.symbol);
            obj.insert("left_mileage_pulse", left_mileage.pulse);
            obj.insert("left_mileage_time", left_mileage.time);
            obj.insert("right_mileage_symbol", right_mileage.symbol);
            obj.insert("right_mileage_pulse", right_mileage.pulse);
            obj.insert("right_mileage_time", right_mileage.time);
            PushCilents(SUBSCRIBE_METHOD(Mileage), obj);
        }
        mileage_count_++;
    }return true;
    case 0xFC://������Ϣ�ϴ� ��������
    {
        LOG_INFO(QString("Received Car key Data:%1").arg(data.toHex().toUpper()));
        quint8 key_value, knob_value;
        stream >> key_value >> knob_value;
        switch (key_value) {//������ֹͣ��������Ŀǰ�����ϴ�
        case 0x02://�������� ��С�����ȼ��ߣ�
        case 0x03://ֹͣ���� ��С�����ȼ��ߣ�
        case 0x04://��ʼɨ�谴�� ��������ȼ��ߣ�
            LOG_INFO("��ʼɨ�谴��");
            break;
        case 0x05://ֹͣɨ�谴�� ��������ȼ��ߣ�
            LOG_INFO("ֹͣɨ�谴��");
            break;
        case 0x0A://�����л���ť
        case 0x0B://ģʽ�л���ť��������ͣ��ť��
        default:
            break;
        }
        PushCilents(SUBSCRIBE_METHOD(CarKey), key_value);
    }return true;
    case 0xFD://ɨ����CANָ���ϴ� ������
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
        case 0x0C://Mirror Index Trigger Occurred���������������ѷ���
            //Out: Automation time of occurred mirror index
        default:
            break;
        }
        //QJsonObject obj;
        //obj.insert("flag", flag);
        //obj.insert("data", data);
        //result = obj;
    }return true;
    case 0xFE://Ϊ�˼��ݵ��������
    {
        MileageInfo mileage_info{};
        stream >> mileage_info.symbol >> mileage_info.pulse >> mileage_info.time;
        mileage_count_++;
        if (mileage_count_ % kMileageUpdateInterval == 0) {
            QJsonObject obj;
            obj.insert("mileage_count", mileage_count_);
            obj.insert("mileage_symbol", mileage_info.symbol);
            obj.insert("mileage_pulse", mileage_info.pulse);
            obj.insert("mileage_time", mileage_info.time);
            PushCilents(SUBSCRIBE_METHOD(Mileage), obj);
        }
    }return true;
#endif // DEVICE_TYPE_CAR
#ifdef DEVICE_TYPE_CLOVER
    case CAMERA_GET_MSG:
    {
        stream >> clover_info_.enable >> clover_info_.mode;
        stream.readRawData(reinterpret_cast<char*>(clover_info_.fps), 10);
        QJsonObject obj;
        obj.insert("enable", clover_info_.enable);
        obj.insert("mode", clover_info_.mode);
        obj.insert("fps", clover_info_.fps[0]);
        //ͳһ,��ÿ��ʹ��
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
    /**�Զ��ϴ������ݸ�ʽ**/
    case 0xF5://���ʱ�������ϴ�
    {
        CloverTriggerInfo trigger{};
        stream >> trigger.count >> trigger.time >> trigger.id >> trigger.feedback >> trigger.position;
        QJsonObject obj;
        obj.insert("count", trigger.count);
        obj.insert("time", trigger.time);
        obj.insert("id", trigger.id);
        obj.insert("feedback", trigger.feedback);
        obj.insert("position", trigger.position);
        PushCilents(SUBSCRIBE_METHOD(CloverTrigger), obj);
    }return true;
#endif // DEVICE_TYPE_CLOVER
    //������,��ȡ�豸����
    case CODE_TEST:
    {
        quint8 type;
        stream >> type;
        result = type;//�ɽ��������豸����
        if ((g_serial_device_type & type) == 0) {
            LOG_ERROR(tr("The device type that is currently set is inconsistent with the device type that is currently supported!"));
        }
        g_serial_device_type = type;
        gSouth.RegisterSettings->setValue("type", type);//ע�����������
    }break;
    default:
        return SerialPortTemplate::HandleProtocol(code, data);
    }
#pragma endregion
    //��ζ�Ӧ�����������?
    SerialSession::instance().HandleTrolleySession(code, result, message);
    return true;
};
