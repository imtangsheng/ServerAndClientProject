/*!
 * @file iController.h
 * @brief 用于声明继承控制类的接口
 */

 /*!
  * @defgroup iController MVC架构的控制模块
  * @brief 处理逻辑模块描述
  */

  /*!
   * @ingroup Controller
   * @brief 负责处理逻辑，连接Model和View
   */

#include "stdafx.h"
#pragma once

#include <QWebSocket>
#include <QThread>
#include "SouthGlobal.h"

using SocketPtr = QWebSocket*;
// 用于ws通信协议参考JSON-RPC 2.0 协议
// 请求模型
struct Session {
    int id{ 0 };///< @brief 请求ID 可选
    int code{ 0 };///< @brief 执行的错误码,非0为执行异常 可选
    SocketPtr socket{ nullptr };///< @brief 请求的socket 可选
    QString module{ "" }; ///< @brief 可选
    QString method;///< @brief 需要调用的函数,槽等 必须
    QJsonValue params;///< @brief  请求参数,如果是一个数组,则反射动态调用槽函数，可以为空;否则调用默认的带完整请求的槽函数 可选
    QJsonValue result;///< @brief 执行的结果 可选
    QVariant context;///< @brief  上下文信息 可选

    // 默认构造函数
    Session() = default; // 默认构造函数
    ~Session() {
        if (socket) delete socket;
    }
    operator bool() const { return code == 0; }
    // 从 JSON 字符串解析为 Request
    Session(QJsonObject json) {
        if (json.contains("id")) id = json["id"].toInt(0);
        if (json.contains("code")) code = json["code"].toInt(-1);
        if (json.contains("module")) module = json["module"].toString("");
        if (json.contains("method")) method = json["method"].toString("");
        if (json.contains("params")) params = json["params"];
        if (json.contains("result")) result = json["result"];
        if (json.contains("context")) context = json["context"].toVariant();
    }

    // 请求的Request发送
    static QString toRequestString(int id, const QString& module, const QString& method, const QJsonValue& params) {
        return jsonToString({ {"id", id}, {"module", module}, {"method", method}, {"params", params} });
    }
    QString toErrorString(int errorCode, const QString& message) const {
        return jsonToString({ {"id", id}, {"code", errorCode}, {"method", method}, {"message", message} });
    }
    QString toResponseString(const QJsonValue& ExecutionResult = QJsonValue(), const QString& ExecutionMessage = QString()) {
        return jsonToString({ {"id", id}, {"code",code}, { "method", method }, {"params", params},{"result", ExecutionResult}, {"message", ExecutionMessage} });
    }
};

///用std::function定义处理器类型 QFunctionPointer 为Qt的函数指针类型无参数无类型返回值;不支持通过字符串动态查找函数。
///不支持跨线程调用（需要手动实现线程安全）。不支持信号槽机制。
///依赖于 Qt 的元对象系统，需要 Q_OBJECT 宏和 moc 预处理。性能开销较大，因为需要通过字符串查找函数。
using SessionHandler = std::function<void(Session&)>;

/*!
 * @var int globalCounter
 * @brief 全局会话默认处理方法,需要注册 使用宏定义自动生成映射
 *
 * @details 使

 * @see SessionHandler
 */
inline QMap<QString, SessionHandler> gSession;
#define REGISTER_HANDLER(name) \
    gSession[#name] = &Class::name

inline void RegisterHandler(const QString& module, const QString& method, SessionHandler handler) {
    gSession[module + "." + method] = handler;
}
 template<typename F>
 void register_handler(const std::string& name, F&& handler) {//“万能引用”（Universal Reference）的语法。它可以绑定到左值或右值引用，具体取决于传入的参数类型。这使得函数可以接受各种类型的参数，包括临时对象和可移动对象。
     gSession[name] = std::forward<F>(handler);// 模板函数，使用 std::forward 完美转发参数
 }

/*!
 * @class Controller
 * @brief 全局的控制类管理模块
 *
 * @details 这是一个控制类，用于控制继承的调用方法。
 */
class Controller : public QObject {
    Q_OBJECT
public:
    static Controller& instance() {
        static Controller instance;
        return instance;
    }
    QMap<QString, QObject*> getHandlers() {
        static QMap<QString, QObject*> handlers;
        return handlers;
    }
    /**
    * @brief 使用宏定义自动生成映射
    * @param message 消息内容
    * @param client 需要发送的客户端
    */
    Result invoke(QJsonObject json, SocketPtr client) {
        Session session(json);
        if (!m_handlers.contains(session.module)) {
            qWarning() << "Module not found:" << session.module;
            return Result(false, session.toErrorString(-1, tr("设备模块未找到:%1").arg(session.module)));
        }
        session.socket = client; //Q_INVOKABLE 使方法可以通过信号与槽机制、或者通过 QMetaObject::invokeMethod 等方式动态调用
        return invoke(session);
    }
    /**
    * @param Session 消息内容的结构体,接收消息结构体
    */
    Result invoke(const Session& session) {
        // 获取目标模块的对象
        QObject* targetObject = m_handlers[session.module];
        const char* method = session.method.toUtf8().constData();
        bool success{ false };// 调用方法目标模块的对象返回
        // 1.使用可变参数模板调用 std::apply(func, std::tuple);需要确定参数个数,std::make_index_sequence<>常量类型故不使用
        if (session.params.isArray()) {// 判断是否为数组
            QJsonArray array = session.params.toArray();
            int paramCount = array.size();
            if (paramCount > 10) { // QMetaObject::invokeMethod 最多支持 10 个参数
                qWarning() << "Too many parameters for invokeMethod!";
                return Result(false, session.toErrorString(-2, tr("最多支持 10 个参数")));
            }
            QGenericArgument* argv = new QGenericArgument[paramCount];  // 动态分配参数数组
            QList<QVariant> tempStorage;  // 用来临时存储数据，避免悬空指针,字符传入失败
            // 遍历请求的参数，并根据类型转换为 QGenericArgument 
            for (int i = 0; i < paramCount; ++i) {
                QJsonValue param = array[i];
                if (param.isString()) {
                    tempStorage.append(param.toString());  // 确保 QString 存活,不然会被释放,其他类型不会
                    //argv[i] = QGenericArgument("QString", reinterpret_cast<const void*>(&tempStorage.last()));
                    argv[i] = QGenericArgument("QString", &tempStorage.last());
                    //argv[i] = Q_ARG(QString, &tempStorage.last().toString());//不可用
                }
                else if (param.isBool()) {
                    tempStorage.append(param.toBool()); argv[i] = QGenericArgument("bool", reinterpret_cast<const void*>(&tempStorage.last()));
                }
                else if (param.isDouble()) {
                    tempStorage.append(param.toDouble()); argv[i] = QGenericArgument("double", &tempStorage.last());
                }
                else if (param.isObject()) {
                    tempStorage.append(param.toObject()); argv[i] = QGenericArgument("QJsonObject", reinterpret_cast<const void*>(&tempStorage.last()));
                }
                else {
                    delete[] argv;  // 释放内存
                    return Result(false, session.toErrorString(-2, tr("发送的参数类型是不支持的参数类型")));
                }
            }
            switch (paramCount) {
            case 0:success = QMetaObject::invokeMethod(targetObject, method); break;
            case 1:success = QMetaObject::invokeMethod(targetObject, method, argv[0]); break;
            case 2:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1]); break;
            case 3:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2]); break;
            case 4:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3]); break;
            case 5:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4]); break;
            case 6:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]); break;
            case 7:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]); break;
            case 8:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]); break;
            case 9:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]); break;
            case 10:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]); break;
            default:
                qFatal() << "Qt is support 10 params,Unsupported number of arguments!";
                break;
            }
            delete[] argv;
        }
        else
        {
            // 默认使用请求对象作为参数 isNull()是否无效或者未定义值 其他参数调用默认 备注:空的""为字符串类型,会判断为有效值
            success = QMetaObject::invokeMethod(targetObject, method, Q_ARG(const Session&, session));
        }
        if (!success)//如果未找到调用的方法,返回失败,支持函数重载
        {
            return Result(false, session.toErrorString(-3, tr("方法调用失败,请检测模块%1的方法:%2使用的参数是否正确").arg(session.module).arg(session.method)));
        }
        return true;
    }
private:
    QMap<QString, QObject*> m_handlers;
    QThread* workerThread{ nullptr };
    Controller() {
        this->setObjectName("ControllerHandlerManager");
        initialize();
    }
    void initialize() {
        workerThread = new QThread();
        workerThread->setObjectName("ControllerHandlerThread");
        // 确保对象移动到目标线程,构造函数本身的代码仍在原始线程（如主线程）中执行，只有对象的信号槽和事件处理会迁移到目标线程。
        this->moveToThread(workerThread);
        workerThread->start();  // 先启动线程
    }
    ~Controller() {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
    }
    Controller(const Controller&) = delete;
    Controller& operator=(const Controller&) = delete;
signals:
    void sigSend(const QString& message, SocketPtr client = nullptr);//多线程的网络发送,需要使用信号连接到统一的线程中发送信息
};
#define gController Controller::instance()

/*!
 * @class ControllerBase
 * @brief 全局的控制类管理模块 interface IControllerSDK wrapper method implementations
 *
 * @details 这是一个控制类，用于控制继承的调用方法。
 * Qt框架设计上就要求parent参数必须是非const的QObject指针,因此传递的this 是非 const 的 QObject 指针是必要的
 *
 */
class ControllerBase : public QObject
{
    Q_OBJECT
public:
    explicit ControllerBase(const QString& module = "", QObject* parent = nullptr)
        : QObject(parent), selfName(module) {// 创建工作线程 对象有父对象时不能移动到其他线程。Qt的父子对象必须在同一个线程中。
        // 自动注册到管理器
        Controller::instance().getHandlers()[selfName] = this;
        initialize();
    }
    virtual ~ControllerBase() {
        if (selfThread) {
            selfThread->quit(); selfThread->wait(); delete selfThread;
        }
    }
    // 派生类可以重写此方法进行线程特定的初始化
    virtual void initialize() {}
public slots:
    //interface IControllerSDK wrapper method implementations
    virtual Result  prepare() { return false; }
    virtual Result start() = 0;
    virtual Result stop() = 0;
    virtual Result pause() { return false; }
    virtual Result shutDown() { return false; }
protected:
    QString selfName;//外部代码无法直接访问
    QThread* selfThread{ nullptr };
    QSet<QWebSocket*> subscribe;
};
