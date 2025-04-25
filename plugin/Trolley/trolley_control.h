/*!
 * @file TrolleyControl.h
 * @brief 小车控制器 ->cn
 * @date 2025-02-24
 */
#pragma once

inline Atomic<bool> direction;
inline Atomic<qint16> speed;
inline double speed_multiplier = 1.0;

class TrolleyControl
{
public:
    TrolleyControl() = default;
    ~TrolleyControl();
    // 定义共享的接口方法
    bool initialize();
    Result SetConfig(const QJsonObject& config);
    Result scan(); // 预先浏览相机设备
    Result open(); //打开相机
    Result close(); //关闭相机
    Result start(); //开始采集
    Result stop(); //停止采集

    QStringList device_id_list;

    QString DeviceName() const {
        static QString name{"Trolley"};
        return name;
    }
    QJsonObject speed_multiplier_json{
        {"MS201", 0.002},
        {"MS301", 0.00001054611773681640625}
    };
protected:
    //用于支持tr的翻译
    static QString tr(const char* sourceText) {
        return QObject::tr(sourceText);
    }
};


