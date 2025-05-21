/*!
 * @file SerialControl.h
 * @brief 小车控制器 ->cn
 * @date 2025-02-24
 */
#pragma once
#include "iSerialPort.h"
using namespace serial;

//向所有ws的客户端推送消息
inline void PushCilents(const QString& method, const QJsonValue& params, const QString& module = g_module_name) {
    emit gSigSent(Session::RequestString(module, method, params));
}
//向指定的ws的客户端发送消息
inline void PushSessionResponse(const Session& session,const QJsonValue& result, const QString& message) {
    emit gSigSent(session.ResponseString(result, message), session.socket);
}

#define g_serial_session SerialSession::instance()
// 定义回调函数类型（支持异步回调函数lambda）
using CallbackFunctionCode = std::function<void(const QJsonValue&, const QString&)>;

//默认的转发函数 智能指针[session=std::make_shared<Session>(session)] 确保即使原始session被销毁，lambda中的session副本仍然有效
static CallbackFunctionCode CallbackDefault(const Session& session) {
    return [&session](const QJsonValue& result, const QString& message) {
        PushSessionResponse(session, result, message);
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
    //QMap<FunctionCodeType, QList<Session>> sessionMap;
    QMap<FunctionCodeType, QList<CallbackFunctionCode>> sessionCallbackMap;
    bool addSession(FunctionCodeType code, const Session& session) {
        QMutexLocker locker(&mutex);
        //if (!sessionMap.contains(code)) {
        //    sessionMap[code] = QList<Session>();
        //}
        //sessionMap[code].append(session);
        return addSession(code, CallbackDefault(session));
    }
    bool addSession(FunctionCodeType code, const CallbackFunctionCode& callback) {
        QMutexLocker locker(&mutex);
        //if (!sessionCallbackMap.contains(code)) {
        //    sessionCallbackMap[code] = QList<CallbackFunctionCode>();
        //}
        sessionCallbackMap[code].append(callback);
        return true;
    }
    void HandleTrolleySession(FunctionCodeType code, QJsonValue result, QString message) {
        //处理回调
        QMutexLocker locker(&mutex);
        if (auto it = sessionCallbackMap.find(code); it != sessionCallbackMap.end()) {//结构化绑定（C++17）
            // it 是迭代器，it->second 是 QList<CallbackFunctionCode>
            for (auto& call : it.value()) {// 这里 it->second 等价于 it.value()
                call(result, message);
            }
            sessionCallbackMap.erase(it);// STL 和 Qt 容器中通过迭代器删除元素的标准方法
        }
        // 处理会话    
        //if (sessionMap.contains(code)) {
        //    for (const Session& session : sessionMap[code]) {
        //        //gShare.on_send(true, session);
        //        emit gSigSent(session.ResponseString(result, message), session.socket);
        //    }
        //    sessionMap.remove(code);
        //}
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

    //任务类的方法,需要回应
    void start(const Session& session);
    void stop(const Session& session);

    //直接调用 使用回调函数在执行特定顺序的任务
    Result OnStarted(CallbackResult callback = nullptr);
    Result OnStopped(CallbackResult callback = nullptr);

protected:
    bool HandleProtocol(FunctionCodeType code, const QByteArray& data) override;

};
