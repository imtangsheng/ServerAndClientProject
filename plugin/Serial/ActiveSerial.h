/*!
 * @file SerialControl.h
 * @brief 小车控制器 ->cn
 * @date 2025-02-24
 */
#pragma once
#include "iSerialPort.h"
using namespace serial;

inline void PushCilents(const QString& method, const QJsonValue& params) {
    emit gSigSent(Session::RequestString(g_module_name, method, params));
}

#define g_serial_session SerialSession::instance()
class SerialSession
{
public:
    static SerialSession& instance() {
        static SerialSession instance;
        return instance;
    }
    QMutex mutex;
    QMap<FunctionCodeType, QList<Session>> sessionMap;
    bool addSession(FunctionCodeType code, const Session& session) {
        QMutexLocker locker(&mutex);
        if (!sessionMap.contains(code)) {
            sessionMap[code] = QList<Session>();
        }
        sessionMap[code].append(session);
        return true;
    }

    void HandleTrolleySession(FunctionCodeType code, QJsonValue result, QString message) {
        // 处理会话    
        if (sessionMap.contains(code)) {
            for (const Session& session : sessionMap[code]) {
                //gSouth.on_send(true, session);
                emit gSigSent(session.ResponseString(result, message), session.socket);
            }
            sessionMap.remove(code);
        }
    }
private:
    SerialSession() = default;
    ~SerialSession() = default;
};

class ActiveSerial : public SerialPortTemplate
{
    Q_OBJECT
public:

    // 继承基类的所有构造函数
    using SerialPortTemplate::SerialPortTemplate;
    //ActiveSerial() = default;
    ~ActiveSerial();

    /*继承基类方法*/
    Result SetConfig(const QJsonObject& config);

    QStringList device_id_list;

    /**设备名称,可以根据设备名称获取对应的资源文件(可避免同名)
    Q_NAMESPACE 必须在命名空间内部  后面必须跟 Q_ENUM_NS 或其他 Qt 元对象宏
    即使用枚举名称,该枚举需要定义在命名空间内部
    */
    static QString DeviceName() {
        //static QMetaEnum metaEnum = QMetaEnum::fromType<SerialDeviceType>();
        //return metaEnum.valueToKey(kSupportedSerialDevices);
        return QVariant::fromValue(kSupportedSerialDevices).toString();
    }


    // 定义共享的任务的接口方法
    Result start(); //开始采集
    Result stop(); //停止采集

protected:
    bool HandleProtocol(FunctionCodeType code, const QByteArray& data) override;

};
