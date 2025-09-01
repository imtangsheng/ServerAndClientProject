/**
 * ������-���� ͨѶ�ֲ� 1.2  ������ 2022-04-18
 * ���ݸ�ʽ��16 ����
 * ����֡��ʽ����8 λ����λ��1 λֹͣλ����У�飬Ĭ�ϲ����� 921600��
 * ��ʶ����2 byte�� 41 54
 * ������ַ ��1 byte�� 64
 * �����루1 byte��
 * ���ݸ�ʽ��16 ����
 * ���ݣ�n byte��
 * CRC У�飨2 byte��
 * ��β��2 byte��0D 0A
 *
 * ��ʶ�����̶�Ϊ 41 54
 * ������ַ���̶�Ϊ 64
 * �����룺ÿ�������в�ͬ�Ĺ�����
 * ���ݣ����ݵĳ��ȿ���Ϊ 0 �� n �ֽڣ�����Ϊ 0 ʱֱ��������һλ
 * CRC У�飺���� crc16 �ķ�ʽ����У�飬У����ֽ�Ϊ CRC ֮ǰ�������ֽڣ����ֽ���ǰ�����ֽ��ں��ֲ��β��˵����
 * ��β���̶�Ϊ 0D 0A
 *
 */
#ifndef _SERIALPORT_PROTOCOL_H
#define _SERIALPORT_PROTOCOL_H

namespace serial {
    typedef unsigned char FunctionCodeType;
    // ֡��ʽ����

    // ��ʶ�� 
    inline static constexpr quint16 HEADER_BYTE = 0x0D0A;    // CR LF
    // ��ʶ�� - ֱ�ӳ�ʼ��Ϊ QByteArray
    static inline const QByteArray HEADER_BYTES = QByteArrayLiteral("\x41\x54");  // 'A''T'
    inline static constexpr quint8 HEADER_BYTE_LEN = 2;
    // ������ַ
    inline static constexpr quint8 LOCAL_ADDRESS = 0x64; //
    // ������
    //FunctionCodeType FUNCTION_CODE = 0x00; //ÿ�������в�ͬ�Ĺ�����
    inline static constexpr quint8 FUNCTION_CODE_POS = 3;//�����ʶ���Ŀ�ʼλ��
    // ����
    inline static constexpr quint8 DATA_POS = 4; // �����ʶ���Ŀ�ʼλ��
    //CRC У��
    inline static constexpr quint16 CRC_BYTE_LEN = 2;
    // ֡β
    inline static constexpr quint16 TAIL_BYTE = 0x0D0A;    // CR LF
    // ֡β - Ҳ��Ϊ QByteArray
    static inline const QByteArray TAIL_BYTES = QByteArrayLiteral("\x0D\x0A");
    inline static constexpr quint8 TAIL_BYTE_LEN = 2;

    // ���� CRC16 У��ֵ
        // CRC16 MODBUS ���ұ�
    inline static constexpr uint16_t CRC_Table[256] = {
        0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
        0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
        0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
        0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
        0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
        0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
        0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
        0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
        0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
        0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
        0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
        0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
        0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
        0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
        0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
        0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
        0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
        0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
        0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
        0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
        0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
        0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
        0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
        0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
        0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
        0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
        0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
        0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
        0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
        0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
        0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
        0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
    };
    inline static QByteArray CRC16(const QByteArray& data) {
        uint16_t crc = 0xFFFF;
        const uint8_t* buf = reinterpret_cast<const uint8_t*>(data.constData());
        int len = data.length();
        // ʹ�ò��ұ����CRC
        while (len--) {
            crc = (crc >> 8) ^ CRC_Table[(crc ^ *buf++) & 0xFF];
        }
        // ת��ΪQByteArray
        QByteArray result(2, 0);
        result[0] = static_cast<char>(crc & 0xFF);         // ���ֽ�
        result[1] = static_cast<char>((crc >> 8) & 0xFF);  // ���ֽ�
        return result;
    }

    //�ظ�ִ�гɹ�
    enum FunctionCodeResultEnum : quint8 {
        RESULT_SUCCESS = 0x00,  // �ɹ�
        RESULT_FAILURE = 0x01,  // ʧ��
    };

    /*������ö��
    * CODE_ ͨ�ÿ�ͷ
    * SCANNER_ɨ����
    * CAR_��С������ָ�� Trolley
    * BATTERY_�ǵ��������
    * LED_ LED�ƹ����
    */
    enum FunctionCodeEnum : FunctionCodeType {
        CODE_TEST = 0x00,  // ����
        CODE_GET_ID = 0x01,  // ��ȡID

        // ɨ����ָ��
        SCANER_END_SCAN = 0x02,  // ����ɨ��
        SCANER_INITIATE_SCAN = 0x03,  // ��ʼ��ɨ��
        SCANER_SET_AUTOMATION_TIME = 0x04,  // �����Զ���ʱ��
        SCANER_RECORD_PAUSE = 0x05,  // ��ʼ/��ͣ��¼ɨ������
        SCANER_DATA_MESSAGE = 0x06,  // ��ɨ���ǵĵ��������м����û�����
        SCANER_SELF_TEST = 0x07,  // �Բ�
        SCANER_GET_AUTOMATION_TIME = 0x08,  // ��ȡɨ���ǵ��Զ���ʱ��
        SCANER_MIRROR_INDEX_TRIGGER_ENABLE = 0x09, // ����
        SCANER_MIRROR_INDEX_TRIGGER_OCCURRED = 0x0a, // ����

        // С������ָ��
        CAR_STARTUP = 0x0b,  // ����С��
        CAR_STOP = 0x0c,  // ֹͣС��
        CAR_CHANGING_OVER = 0x0d,  // ����
        CAR_SET_SPEED = 0x0e,  // �����ٶ�
        CAR_RESET_TOTAL_MILEAGE = 0x0f,  // ���С���������
        CAR_GET_MSG = 0x10,  // ��ȡС������Ϣ
        // ��Դ����
        CAR_SCANER_PWR_CTRL = 0x11,  // ɨ���ǵ�Դ����
        CAR_PWR_CTRL = 0x12,  // �����Դ����
        CAR_GET_TOTAL_MILEAGE = 0x13,  // ��ȡ�����
        // ���ָ��
        CAR_GET_BATTERY_MSG = 0x14,  // ��ȡ�����Ϣ
        CAR_SET_ENCODER_USAGE = 0x15,  // ����ʹ���Ǹ�������
        CAR_SET_START_STOP_BUTTON_MODE = 0x16,  // ������ͣ����ģʽ
        CAR_SET_EMERGENCY_STOP_MODE = 0x17,  // �����Ƿ����ü�ͣ��ť
        CAR_GET_EMERGENCY_STOP_MODE = 0x18,  // ��ȡ�Ƿ����ü�ͣ��ť
        CAR_SWITCH_VOLTAGE_CTRL = 0x19,  // �л���ѹ
        CAR_CHOOSE_BATTERY_SOURCE = 0x1a,  // ����ʹ����һ���ع���

        // Camera commands
        CAMERA_GET_MSG = 0x30,                         /* ��ȡ���õ���Ϣ */
        CAMERA_SET_FREQUENCY = 0x31,                   /* �����������Ƶ�� */
        CAMERA_START_STOP = 0x32,                      /* ��ʼ��ֹͣ���� */
        CAMERA_SET_ENABLE_MODE = 0x33,                 /* �������ʹ�ܺ͹���ģʽ */
        CAMERA_LED_CONTROL = 0x34,                     /* LED���� */
        CAMERA_FAN_CONTROL = 0x35,                     /* ���ȿ��� */
        CAMERA_GET_LED_FAN_STATUS = 0x36,              /* ��ȡLED�ͷ��ȿ���״̬ */
        CAMERA_GET_TEMP_VOLTAGE = 0x37,                /* ��ȡ�¶ȡ���ѹ����������� */
        CAMERA_MOTOR_CONTROL = 0x38,                   /* ����������� */

        CAMERA_SET_UNIQUE_ID = 0x39,                   /* ���������Ψһ��ţ�ID��Χ0-255 */
        CAMERA_GET_UNIQUE_ID = 0x3A,                   /* ��ȡ�����Ψһ��� */
        CAMERA_SET_UNIQUE_ENABLE_MODE = 0x3B,          /* ��Ψһ�����ʽ�������ʹ�ܺ͹���ģʽ */
        CAMERA_SET_UNIQUE_FREQUENCY = 0x3C,            /* ��Ψһ�����ʽ�����������Ƶ�� */
        CAMERA_START_STOP_UNIQUE = 0x3D,               /* ��Ψһ�����ʽ��ʼ��ֹͣ���� */

        CAMERA_MOTOR_SEARCH_ZERO = 0x3E,               /* �������������� */
        CAMERA_START_UNIQUE_ONE_FRAME = 0x3F,          /* ��ʼ����һ֡ */
        CAMERA_SET_EXPOSURE_TIME = 0x40,               /* �����ع�ʱ�� */
        CAMERA_M2_FAN_CONTROL = 0x41,                  /* M2ɢ�ȷ��ȿ��� */
        CAMERA_MOTOR_SPEED_CONTROL = 0x42,             /* ��ת���ֵĵ��ת�ٺͷ������ */
        CAMERA_MOTOR_START_STOP_CONTROL = 0x43,        /* �����ͣ���� */
        CAMERA_READ_MOTOR_STATUS = 0x44,               /* ��ȡ���״̬ */
        CAMERA_SET_MOTOR_POSITION = 0x45,              /* ��ת��ָ��λ��1-15 */
        CAMERA_SET_TRIGGER_POSITION = 0x46,            /* �������մ���λ�� */
        CAMERA_GET_TRIGGER_POSITION = 0x47,            /* ��ȡ���մ���λ�� */

        CAMERA_SET_GET_BM1_LOCATION_VALUE = 0x48,      /* ��ȡ������λ��1���ڵı�����ֵ */
        CAMERA_SET_GET_TRIGGER_MODE = 0x49             /* �򿪻�رչ�翪���쳣��ⴥ��ģʽ�Զ��л� */
    };

    /*������������෽��*/
    Q_NAMESPACE
        enum SerialDeviceType : quint8 {
        SERIAL_NONE = 0x00,//���豸
        SERIAL_CAR = 0x01,//С���п���
        SERIAL_CAMERA = 0x02,//����豸
        SERIAL_CAR_CAMERA = SERIAL_CAR | SERIAL_CAMERA
    };
    Q_ENUM_NS(SerialDeviceType)

#pragma region serialport
        //�̶���ģ������
        inline static const QString g_module_name = share::Shared::GetModuleName(share::ModuleName::serial);

#define Function_Code(name) def_##name(const QByteArray& data)
#define DefineFunction(name) funtion##name(const QByteArray& data)
#define SUBSCRIBE_FUNCTION(name) on##name##Change
#define SUBSCRIBE_METHOD(name) "on" #name "Change"

    inline QString FunctionCodeToString(FunctionCodeType code) {
        //qUtf8Printable;//qUtf8Printable �������������������ڱ����ַ������������ڣ����Ǹ���ȫ������
        //QString::number(code, 16) ������ code ת��Ϊָ�����ƣ������� 16 ���ƣ����ַ���
        //.rightJustified(2, '0') ���ã����ַ����Ҷ��룬ȷ���䳤������Ϊָ����ȣ������� 2����������㣬����������ָ�����ַ��������� '0'��
        //.toUpper() ���ã����ַ����е�������ĸת��Ϊ��д��ʮ�������е���ĸ a - f ��Ϊ A - F�����ֺ������ַ����䡣
        return QString("function0x") + QString::number(code, 16).rightJustified(2, '0').toUpper();
    }

#pragma endregion

#ifdef DEVICE_TYPE_CAR
    /*С�����ݸ�ʽ����*/
    struct CarInfo
    {
        //С����Ϣ ָ��ظ�
        uint16_t speed;
        uint8_t temperature_symbol; //0 Ϊ����1 Ϊ��
        uint16_t temperature;//��λΪ0.1�� ��ʾʱ��Ҫ����10
        uint8_t direction; // ���� 0 ���� 1 ǰ��
        uint8_t moving; // ��ͣ״̬ 0 ֹͣ 1 ����
        uint8_t battery_usage;//���ʹ��״̬��0 δ��ʼ�� ��أ�1 ʹ������أ� 2 ʹ���Ҳ���
        uint8_t knob_state;//��ť״̬��1 �� 5 �ֱ��ʾ��ť��״̬ 1 �� 5��
        uint8_t encoder_mode;//ΪĿǰʹ�õ����ĸ���������	1 ��࣬2 �Ҳ࣬3 ͬʱʹ������������ Ŀǰ��˫������
        uint8_t key_mode;//Ϊ����ģʽ��0 С�����ȼ��ߣ����°���С��ֱ��������1 ������ȼ��ߣ�С��������������ֻ���Ͱ�����Ϣ������
        uint8_t scanner_power;//Ϊɨ���ǹ���״̬��0 �����磨Ĭ�ϣ���1 ���硣���� 0x11 ָ�����
        uint16_t speed_min; //Ϊ��С�ٶ�
        uint16_t speed_max;
        int64_t time;//��·��ʱ�䣻��λΪ��10 ΢�룩�������������Ը�Ϊ 1 ΢�롣
    };
    struct ScannerAndCarTimeInfo
    {
        quint64 scanner{ 0 };        //ɨ�����Զ���ʱ�� ��λ΢��
        quint64 car{ 0 };              //С�����Զ���ʱ�� ��λΪ10΢��
        quint64 difference{ 0 };
        void upadate(quint64 scanner_time, quint64 car_time) {
            scanner = scanner_time;
            car = car_time;
            difference = scanner - car;
        }
        quint64 GetScanner(quint64 cartime) const {
            return cartime + difference;
        }

    };
    struct BatteryInfo {
        uint8_t state; // 00 �����������쳣
        uint16_t voltage; // in mV Ϊ��ص�ѹ
        int16_t current; // in mA Ϊ��ص���
        uint16_t remainingCapacity; // in mAh Ϊ���ʣ������
        uint16_t fullChargeCapacity; // in mAh Ϊ�����������
        uint16_t cycleCount; // Ϊ���ѭ������
    };
    struct MileageInfo
    {
        quint8 symbol; //Ϊ��̷��ţ�00 Ϊ����01 Ϊ����
        qint64 pulse;//�������� 8 �ֽ�Ϊ��̵���������ʵ�����= ������ * ���岽��Pulse step size�� ��Ҫ ���� speed_multiplier
        qint64 time;//��� ��ʱ��
        MileageInfo() : symbol(0), pulse(0), time(0) {}
        MileageInfo(quint8 symbol,qint64 pulse,qint64 time) : symbol(symbol), pulse(pulse), time(time) {}
        friend QDataStream& operator<<(QDataStream& out, const MileageInfo& info) {
            // ��˳��д�����г�Ա����
            out << info.symbol << info.pulse << info.time;
            return out;
        }
        friend QDataStream& operator>>(QDataStream& in, MileageInfo& info) {
            // ����ͬ˳���ȡ���г�Ա����
            in >> info.symbol >> info.pulse >> info.time;
            return in;
        }

    };

    struct InclinometerInfo { //��ǼƵ�����Ϊ���� 15 ��
        qint32 x = 0;
        qint32 y = 0;
        qint64 time = 0;
    };

    /*���������,���ڻ�������*/
    static inline ScannerAndCarTimeInfo gScannerCarTimeSync;
    static inline qint64 gGetScannerTimeCount{ 0 };//����

    static inline QAtomicInteger<quint64> g_mileage_count{ 0 };//��̼���

    inline static constexpr int kMileageUpdateInterval = 20;//�����еĸ������ʹ���
    inline double g_mileage_multiplier = 1.0;//�ٶȱ���

    inline const QJsonObject MILEAGE_MULTIPLIER{
        {"MS201", 0.0000843689430},
        {"MS301", 0.00001054611773681640625}
    };

    inline static constexpr SerialDeviceType kSupportSerialCar = SERIAL_CAR;
#else
    inline static constexpr SerialDeviceType kSupportSerialCar = SERIAL_NONE;
#endif // DEVICE_TYPE_CAR


#ifdef DEVICE_TYPE_CAMERA
    /*��Ҷ��������ݸ�ʽ*/
    struct CloverInfo
    {
        quint16 enable;//���ʹ��
        quint16 mode;//���ģʽ
        char fps[10];//���Ƶ�� 10 �ֽ� AcquisitionFrameRate
        quint16 led_state;//�����״̬�����ֽ���ǰ�����ֽ��ں󡣺ϳ�֮�� �ӵ�λ����λ�ֱ���� 10�鲹��Ƶ�״̬��
        quint16 fan_state;//����ɢ�ȷ���״̬���벹��Ƹ�ʽ��ͬ��δ���á�
        quint16 speed;//Ϊת��  ��λ����תÿ����
        quint8 direction;//��ʾ��ת����0 ��ʾ��ǰ����1 ��ʾ�෴���򡣸�λĿǰ���ɵ���Ĭ��Ϊ 0��
        quint8 moving;//��ʾ��ͣ״̬��0 ֹͣ��1 ת��
        quint8 calibrated;//��ʾ�������Ƿ��Ѿ�У׼��0��У׼��1 δУ׼����δУ׼����ת��������ת�����л��Զ�У׼��
        quint8 pose;//��ת����λ�ã���Χ 1-15��δУ׼��ֵ��׼ȷ��
        quint16 trigger_position[3];//�����λ���� 15 λ���δ������̵� 1 �� 15λ�á�
    };

    //��Ҷ�����Ĭ����15����λ ����һ̨�豸��21��λ
#ifndef Clover_Positions_Number
#define Clover_Positions_Number 15
#endif

    struct CloverTriggerInfo
    {
        qint32 id; //    �������������
        qint64 time; //   ʱ��
        quint8 camera_id;//    ��� ID
        quint8 feedback;//    �عⷴ����0 �޷�����1 �з���
        quint8 position;//    ��ת����λ�ã�ȡֵ 1 - 15 ��תλ�ò���ʵ�ʶ�Ӧ�Ļ�λ
        quint8 GetActualPosition() const { //��Ӧ��ʵ�ʻ�λ
            quint8 actual_position = 0;
            switch (camera_id) {
            case 1:actual_position = position; break;
            case 2:actual_position = position + Clover_Positions_Number / 3; break;
            case 3:actual_position = position + Clover_Positions_Number / 3 * 2; break;
            default:LOG_ERROR(QObject::tr("Invalid camera ID: %1").arg(camera_id)); actual_position = position; break;
            }
            if (actual_position > Clover_Positions_Number) actual_position -= Clover_Positions_Number;
            return actual_position;
        }
    };

    static inline CloverInfo clover_info_;
    inline static constexpr SerialDeviceType kSupportSerialClover = SERIAL_CAMERA;
#else
    inline static constexpr SerialDeviceType kSupportSerialClover = SERIAL_NONE;
#endif
}//end namespace serial
#endif