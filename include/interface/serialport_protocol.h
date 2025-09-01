/**
 * 多面阵-车载 通讯手册 1.2  电子组 2022-04-18
 * 数据格式：16 进制
 * 数据帧格式：（8 位数据位，1 位停止位，无校验，默认波特率 921600）
 * 标识符（2 byte） 41 54
 * 本机地址 （1 byte） 64
 * 功能码（1 byte）
 * 数据格式：16 进制
 * 数据（n byte）
 * CRC 校验（2 byte）
 * 结尾（2 byte）0D 0A
 *
 * 标识符：固定为 41 54
 * 本机地址：固定为 64
 * 功能码：每个功能有不同的功能码
 * 数据：数据的长度可以为 0 到 n 字节，长度为 0 时直接跳过这一位
 * CRC 校验：采用 crc16 的方式进行校验，校验的字节为 CRC 之前的所有字节，低字节在前，高字节在后；手册结尾另附说明。
 * 结尾：固定为 0D 0A
 *
 */
#ifndef _SERIALPORT_PROTOCOL_H
#define _SERIALPORT_PROTOCOL_H

namespace serial {
    typedef unsigned char FunctionCodeType;
    // 帧格式定义

    // 标识符 
    inline static constexpr quint16 HEADER_BYTE = 0x0D0A;    // CR LF
    // 标识符 - 直接初始化为 QByteArray
    static inline const QByteArray HEADER_BYTES = QByteArrayLiteral("\x41\x54");  // 'A''T'
    inline static constexpr quint8 HEADER_BYTE_LEN = 2;
    // 本机地址
    inline static constexpr quint8 LOCAL_ADDRESS = 0x64; //
    // 功能码
    //FunctionCodeType FUNCTION_CODE = 0x00; //每个功能有不同的功能码
    inline static constexpr quint8 FUNCTION_CODE_POS = 3;//距离标识符的开始位置
    // 数据
    inline static constexpr quint8 DATA_POS = 4; // 距离标识符的开始位置
    //CRC 校验
    inline static constexpr quint16 CRC_BYTE_LEN = 2;
    // 帧尾
    inline static constexpr quint16 TAIL_BYTE = 0x0D0A;    // CR LF
    // 帧尾 - 也改为 QByteArray
    static inline const QByteArray TAIL_BYTES = QByteArrayLiteral("\x0D\x0A");
    inline static constexpr quint8 TAIL_BYTE_LEN = 2;

    // 计算 CRC16 校验值
        // CRC16 MODBUS 查找表
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
        // 使用查找表计算CRC
        while (len--) {
            crc = (crc >> 8) ^ CRC_Table[(crc ^ *buf++) & 0xFF];
        }
        // 转换为QByteArray
        QByteArray result(2, 0);
        result[0] = static_cast<char>(crc & 0xFF);         // 低字节
        result[1] = static_cast<char>((crc >> 8) & 0xFF);  // 高字节
        return result;
    }

    //回复执行成功
    enum FunctionCodeResultEnum : quint8 {
        RESULT_SUCCESS = 0x00,  // 成功
        RESULT_FAILURE = 0x01,  // 失败
    };

    /*功能码枚举
    * CODE_ 通用开头
    * SCANNER_扫描仪
    * CAR_是小车控制指令 Trolley
    * BATTERY_是电池命令类
    * LED_ LED灯光控制
    */
    enum FunctionCodeEnum : FunctionCodeType {
        CODE_TEST = 0x00,  // 测试
        CODE_GET_ID = 0x01,  // 获取ID

        // 扫描仪指令
        SCANER_END_SCAN = 0x02,  // 结束扫描
        SCANER_INITIATE_SCAN = 0x03,  // 初始化扫描
        SCANER_SET_AUTOMATION_TIME = 0x04,  // 设置自动化时间
        SCANER_RECORD_PAUSE = 0x05,  // 开始/暂停记录扫描数据
        SCANER_DATA_MESSAGE = 0x06,  // 向扫描仪的点云数据中加入用户数据
        SCANER_SELF_TEST = 0x07,  // 自测
        SCANER_GET_AUTOMATION_TIME = 0x08,  // 获取扫描仪的自动化时间
        SCANER_MIRROR_INDEX_TRIGGER_ENABLE = 0x09, // 保留
        SCANER_MIRROR_INDEX_TRIGGER_OCCURRED = 0x0a, // 保留

        // 小车控制指令
        CAR_STARTUP = 0x0b,  // 启动小车
        CAR_STOP = 0x0c,  // 停止小车
        CAR_CHANGING_OVER = 0x0d,  // 换向
        CAR_SET_SPEED = 0x0e,  // 设置速度
        CAR_RESET_TOTAL_MILEAGE = 0x0f,  // 清除小车的总里程
        CAR_GET_MSG = 0x10,  // 获取小车的信息
        // 电源控制
        CAR_SCANER_PWR_CTRL = 0x11,  // 扫描仪电源控制
        CAR_PWR_CTRL = 0x12,  // 电机电源控制
        CAR_GET_TOTAL_MILEAGE = 0x13,  // 获取总里程
        // 电池指令
        CAR_GET_BATTERY_MSG = 0x14,  // 获取电池信息
        CAR_SET_ENCODER_USAGE = 0x15,  // 设置使用那个编码器
        CAR_SET_START_STOP_BUTTON_MODE = 0x16,  // 设置启停按键模式
        CAR_SET_EMERGENCY_STOP_MODE = 0x17,  // 设置是否启用急停按钮
        CAR_GET_EMERGENCY_STOP_MODE = 0x18,  // 获取是否启用急停按钮
        CAR_SWITCH_VOLTAGE_CTRL = 0x19,  // 切换电压
        CAR_CHOOSE_BATTERY_SOURCE = 0x1a,  // 设置使用哪一块电池供电

        // Camera commands
        CAMERA_GET_MSG = 0x30,                         /* 获取设置的信息 */
        CAMERA_SET_FREQUENCY = 0x31,                   /* 设置相机拍照频率 */
        CAMERA_START_STOP = 0x32,                      /* 开始或停止拍照 */
        CAMERA_SET_ENABLE_MODE = 0x33,                 /* 设置相机使能和工作模式 */
        CAMERA_LED_CONTROL = 0x34,                     /* LED控制 */
        CAMERA_FAN_CONTROL = 0x35,                     /* 风扇控制 */
        CAMERA_GET_LED_FAN_STATUS = 0x36,              /* 获取LED和风扇开闭状态 */
        CAMERA_GET_TEMP_VOLTAGE = 0x37,                /* 获取温度、电压、电池组数量 */
        CAMERA_MOTOR_CONTROL = 0x38,                   /* 步进电机控制 */

        CAMERA_SET_UNIQUE_ID = 0x39,                   /* 设置相机的唯一编号，ID范围0-255 */
        CAMERA_GET_UNIQUE_ID = 0x3A,                   /* 获取相机的唯一编号 */
        CAMERA_SET_UNIQUE_ENABLE_MODE = 0x3B,          /* 以唯一编号形式设置相机使能和工作模式 */
        CAMERA_SET_UNIQUE_FREQUENCY = 0x3C,            /* 以唯一编号形式设置相机拍照频率 */
        CAMERA_START_STOP_UNIQUE = 0x3D,               /* 以唯一编号形式开始或停止拍照 */

        CAMERA_MOTOR_SEARCH_ZERO = 0x3E,               /* 步进电机搜索零点 */
        CAMERA_START_UNIQUE_ONE_FRAME = 0x3F,          /* 开始拍照一帧 */
        CAMERA_SET_EXPOSURE_TIME = 0x40,               /* 设置曝光时间 */
        CAMERA_M2_FAN_CONTROL = 0x41,                  /* M2散热风扇控制 */
        CAMERA_MOTOR_SPEED_CONTROL = 0x42,             /* 旋转部分的电机转速和方向控制 */
        CAMERA_MOTOR_START_STOP_CONTROL = 0x43,        /* 电机启停控制 */
        CAMERA_READ_MOTOR_STATUS = 0x44,               /* 读取电机状态 */
        CAMERA_SET_MOTOR_POSITION = 0x45,              /* 旋转到指定位置1-15 */
        CAMERA_SET_TRIGGER_POSITION = 0x46,            /* 设置拍照触发位置 */
        CAMERA_GET_TRIGGER_POSITION = 0x47,            /* 获取拍照触发位置 */

        CAMERA_SET_GET_BM1_LOCATION_VALUE = 0x48,      /* 获取或设置位置1所在的编码器值 */
        CAMERA_SET_GET_TRIGGER_MODE = 0x49             /* 打开或关闭光电开关异常检测触发模式自动切换 */
    };

    /*串口任务控制类方法*/
    Q_NAMESPACE
        enum SerialDeviceType : quint8 {
        SERIAL_NONE = 0x00,//无设备
        SERIAL_CAR = 0x01,//小车中控箱
        SERIAL_CAMERA = 0x02,//相机设备
        SERIAL_CAR_CAMERA = SERIAL_CAR | SERIAL_CAMERA
    };
    Q_ENUM_NS(SerialDeviceType)

#pragma region serialport
        //固定的模块名称
        inline static const QString g_module_name = share::Shared::GetModuleName(share::ModuleName::serial);

#define Function_Code(name) def_##name(const QByteArray& data)
#define DefineFunction(name) funtion##name(const QByteArray& data)
#define SUBSCRIBE_FUNCTION(name) on##name##Change
#define SUBSCRIBE_METHOD(name) "on" #name "Change"

    inline QString FunctionCodeToString(FunctionCodeType code) {
        //qUtf8Printable;//qUtf8Printable 宏会在整个语句作用域内保持字符串的生命周期，这是更安全的做法
        //QString::number(code, 16) 将整数 code 转换为指定进制（这里是 16 进制）的字符串
        //.rightJustified(2, '0') 作用：将字符串右对齐，确保其长度至少为指定宽度（这里是 2），如果不足，则在左侧填充指定的字符（这里是 '0'）
        //.toUpper() 作用：将字符串中的所有字母转换为大写。十六进制中的字母 a - f 变为 A - F，数字和其他字符不变。
        return QString("function0x") + QString::number(code, 16).rightJustified(2, '0').toUpper();
    }

#pragma endregion

#ifdef DEVICE_TYPE_CAR
    /*小车数据格式定义*/
    struct CarInfo
    {
        //小车信息 指令回复
        uint16_t speed;
        uint8_t temperature_symbol; //0 为正，1 为负
        uint16_t temperature;//单位为0.1° 显示时需要除以10
        uint8_t direction; // 方向 0 后退 1 前进
        uint8_t moving; // 启停状态 0 停止 1 运行
        uint8_t battery_usage;//电池使用状态；0 未初始化 电池，1 使用左侧电池， 2 使用右侧电池
        uint8_t knob_state;//旋钮状态；1 到 5 分别表示旋钮的状态 1 到 5。
        uint8_t encoder_mode;//为目前使用的是哪个编码器；	1 左侧，2 右侧，3 同时使用两个编码器 目前用双编码器
        uint8_t key_mode;//为按键模式；0 小车优先级高，按下按键小车直接启动；1 软件优先级高，小车并不会启动，只发送按键信息到电脑
        uint8_t scanner_power;//为扫描仪供电状态；0 不供电（默认），1 供电。（由 0x11 指令控制
        uint16_t speed_min; //为最小速度
        uint16_t speed_max;
        int64_t time;//电路板时间；单位为（10 微秒）。如果有需求可以改为 1 微秒。
    };
    struct ScannerAndCarTimeInfo
    {
        quint64 scanner{ 0 };        //扫描仪自动化时间 单位微秒
        quint64 car{ 0 };              //小车的自动化时间 单位为10微秒
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
        uint8_t state; // 00 正常，其他异常
        uint16_t voltage; // in mV 为电池电压
        int16_t current; // in mA 为电池电流
        uint16_t remainingCapacity; // in mAh 为电池剩余容量
        uint16_t fullChargeCapacity; // in mAh 为电池满充容量
        uint16_t cycleCount; // 为电池循环次数
    };
    struct MileageInfo
    {
        quint8 symbol; //为里程符号，00 为正，01 为负。
        qint64 pulse;//接下来的 8 字节为里程的脉冲数。实际里程= 脉冲数 * 脉冲步长Pulse step size。 即要 乘以 speed_multiplier
        qint64 time;//里程 的时间
        MileageInfo() : symbol(0), pulse(0), time(0) {}
        MileageInfo(quint8 symbol,qint64 pulse,qint64 time) : symbol(symbol), pulse(pulse), time(time) {}
        friend QDataStream& operator<<(QDataStream& out, const MileageInfo& info) {
            // 按顺序写入所有成员变量
            out << info.symbol << info.pulse << info.time;
            return out;
        }
        friend QDataStream& operator>>(QDataStream& in, MileageInfo& info) {
            // 按相同顺序读取所有成员变量
            in >> info.symbol >> info.pulse >> info.time;
            return in;
        }

    };

    struct InclinometerInfo { //倾角计的量程为正负 15 度
        qint32 x = 0;
        qint32 y = 0;
        qint64 time = 0;
    };

    /*定义的数据,用于缓存数据*/
    static inline ScannerAndCarTimeInfo gScannerCarTimeSync;
    static inline qint64 gGetScannerTimeCount{ 0 };//计数

    static inline QAtomicInteger<quint64> g_mileage_count{ 0 };//里程计数

    inline static constexpr int kMileageUpdateInterval = 20;//界面中的更新推送次数
    inline double g_mileage_multiplier = 1.0;//速度倍率

    inline const QJsonObject MILEAGE_MULTIPLIER{
        {"MS201", 0.0000843689430},
        {"MS301", 0.00001054611773681640625}
    };

    inline static constexpr SerialDeviceType kSupportSerialCar = SERIAL_CAR;
#else
    inline static constexpr SerialDeviceType kSupportSerialCar = SERIAL_NONE;
#endif // DEVICE_TYPE_CAR


#ifdef DEVICE_TYPE_CAMERA
    /*三叶草相机数据格式*/
    struct CloverInfo
    {
        quint16 enable;//相机使能
        quint16 mode;//相机模式
        char fps[10];//相机频率 10 字节 AcquisitionFrameRate
        quint16 led_state;//补光灯状态，低字节在前，高字节在后。合成之后 从低位到高位分别代表 10组补光灯的状态。
        quint16 fan_state;//代表散热风扇状态，与补光灯格式相同。未启用。
        quint16 speed;//为转速  单位多少转每分钟
        quint8 direction;//表示旋转方向，0 表示当前方向，1 表示相反方向。该位目前不可调，默认为 0。
        quint8 moving;//表示启停状态，0 停止，1 转动
        quint8 calibrated;//表示编码器是否已经校准，0已校准，1 未校准。若未校准，旋转机构在旋转过程中会自动校准。
        quint8 pose;//旋转机构位置，范围 1-15；未校准该值不准确。
        quint16 trigger_position[3];//由最低位到第 15 位依次代表码盘的 1 到 15位置。
    };

    //三叶草相机默认是15个机位 还有一台设备是21机位
#ifndef Clover_Positions_Number
#define Clover_Positions_Number 15
#endif

    struct CloverTriggerInfo
    {
        qint32 id; //    相机触发次数，
        qint64 time; //   时间
        quint8 camera_id;//    相机 ID
        quint8 feedback;//    曝光反馈，0 无反馈，1 有反馈
        quint8 position;//    旋转机构位置，取值 1 - 15 旋转位置不是实际对应的机位
        quint8 GetActualPosition() const { //对应的实际机位
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