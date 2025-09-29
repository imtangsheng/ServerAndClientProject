/*!
 * @file SerialControl.h
 * @brief 小车控制器 ->cn
 * @date 2025-02-24
 */
#pragma once
#include "iSerialPort.h"
using namespace serial;

#define g_serial_session SerialSession::instance()

//默认的转发函数 智能指针[session=std::make_shared<Session>(session)] 共享所有权,确保即使原 session被销毁，lambda中的 session副本仍然有效
static CallbackResult CallbackDefault(const Session& session) {
    //必须保证 lambda 内部捕获的对象生命周期足够长，或者用智能指针捕获对象副本。
    //return *std::make_shared<CallbackResult>(
    //    [session = std::make_shared<Session>(session)](const QJsonValue& result, const QString& message) {
    //        PushSessionResponse(*session, result, message);
    //    }
    //);
    return [session = std::make_shared<Session>(session)](const qint8 &result,const QJsonValue& value) {
        PushSessionResponse(*session, result,value);
    };
}
class SerialSession
{
public:
    static SerialSession& instance() {
        static SerialSession instance;
        return instance;
    }
    QMutex mutex;
    //容器存储智能指针的 std::function 没有意义,只保留最新的会话回调函数指针(使用智能指针副本，保证对象生命周期)。
    QMap<FunctionCodeType, CallbackResult> sessionCallbackMap;
    bool addSession(FunctionCodeType code, const Session& session) {
        QMutexLocker locker(&mutex);
        return AddSessionCallbackImpl(code, CallbackDefault(session));// !这里没有锁,但被第一个函数调用时，锁仍然持有
    }
    bool addSession(FunctionCodeType code, const CallbackResult& callback) {
        QMutexLocker locker(&mutex);
        return AddSessionCallbackImpl(code, callback);// 调用私有实现
    }
    void HandleSessionCallback(FunctionCodeType code,qint8 flag, QJsonValue result) {
        //处理回调
        QMutexLocker locker(&mutex);
        if (sessionCallbackMap.contains(code)) {
            CallbackResult callback = sessionCallbackMap[code];//拷贝，然后删除(引用不能)
            sessionCallbackMap.remove(code);
            locker.unlock();// 释放锁后再执行回调 执行回调（无锁状态）
            // 添加调试信息
            try {
                if (callback) {  // 检查 callback 是否为 nullptr
                    callback(flag, result);
                } else {
                    qWarning() << "[#SerialSession]处理回调Callback is null!";
                }
            } catch (const std::exception& e) {
                qWarning() << "[#SerialSession]处理回调Exception caught during callback execution" << e.what();
            }
        }
    }
private:
    SerialSession() = default;
    ~SerialSession() = default;
    bool AddSessionCallbackImpl(FunctionCodeType code, const CallbackResult& callback) {
        // 实际的实现，由公有函数加锁后调用
        sessionCallbackMap[code] = callback;//QSharedPointer<CallbackResult>::create(callback);
        return true;
    }
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

    /**设备名称,可以根据设备名称获取对应的资源文件(可避免同名)
    Q_NAMESPACE 必须在命名空间内部  后面必须跟 Q_ENUM_NS 或其他 Qt 元对象宏
    即使用枚举名称,该枚举需要定义在命名空间内部
    */
    QString DeviceName() {
        //static QMetaEnum metaEnum = QMetaEnum::fromType<SerialDeviceType>();
        //return metaEnum.valueToKey(kSupportedSerialDevices);
        return QVariant::fromValue(static_cast<SerialDeviceType>(uCurrentSerialDevice)).toString();
    }

    //任务类的方法,需要回应
    void start(const Session& session);
    void stop(const Session& session);

    //直接调用 使用回调函数在执行特定顺序的任务
    Result OnStarted(const CallbackResult& callback = nullptr);
    Result OnStopped(const CallbackResult& callback = nullptr);

protected:
    bool HandleProtocol(FunctionCodeType code, const QByteArray& data) override;

};
