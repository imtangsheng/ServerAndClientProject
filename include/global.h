/**
 * @file global.h
 * @brief 全局文件,常用的内联函数,函数返回类型,网络通信类型等数据结构类型进行定义
 * @author Tang
 * @date 2025-03-10
 */
#ifndef GLOBAL_H_
#define GLOBAL_H_
#include <QtCore/qglobal.h>
#include <QObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>

//公有类QObject需要声明,否则需要显示包含
#if defined(SHAREDLIB_API)
#define SHAREDLIB_EXPORT Q_DECL_EXPORT
#else
#define SHAREDLIB_EXPORT Q_DECL_IMPORT
#endif

inline static QJsonObject StringToJson(const QString& jsonString) {
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
    if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        return jsonDoc.object();
    } else {
        return QJsonObject();
    }
}

// 内联函数，将JSON对象转换为字符串
inline static QString JsonToString(const QJsonObject& jsonObject) {
    QJsonDocument jsonDoc(jsonObject);
    return QString::fromUtf8(jsonDoc.toJson());
}


/**定义一个结构体来包含更详细的结果信息**/
struct Result
{
    int code{ -1 }; //错误码,0为成功
    QString message{ "" };
    Result(int i, const QString& msg = "") :code(i), message(msg) {}
    Result(bool s = true, const QString& msg = "") :message(msg) {
        if (s) { code = 0; }
    }//隐式构造函数调用
    static Result Success(const QString& msg = "") { return Result(true, msg); }
    static Result Failure(const QString& msg = "") { return Result(false, msg); }
    operator bool() const { return code == 0; }//重载了 bool 操作符，使其可以像之前的 bool 返回值一样使用例如：if (result)

};
// 声明结构体为元类型
Q_DECLARE_METATYPE(Result)
// 注册 结构体为元类型 运行时环境初始化时调用  main 函数或者构造函数中
//qRegisterMetaType<Result>("Result");

// 定义回调函数类型（支持异步回调函数lambda）
using CallbackResult = std::function<void(Result)>;

/**定义一个原子类的结构体**/
#include <QAtomicInteger>
template<typename T>
struct Atomic
{
    QAtomicInteger<T> value;
    QString message;
    // Relaxed - 最宽松的内存序
    // Release - 确保之前的写入对其他线程可见
    // Acquire - 确保之后的读取能看到其他线程的Release写入
    // Ordered - 完全内存屏障
    T Get() const { return value.loadAcquire(); }
    void Set(T s) { value.storeRelease(s); }
    //在构造函数中，可以直接用 T 类型的值初始化它
    Atomic(T t = T(), const QString& msg = QString()) : value(t), message(msg) {}
    // 重载赋值运算符，允许直接赋值
    Atomic& operator=(T value) {
        Set(value);
        return *this;
    }
    // 重载类型转换运算符，允许隐式转换为 T 类型
    operator T() const { return Get(); }

    // 支持隐式转换
    operator Result() const {
        return Result(Get(), message);
    }

};
#include <QAtomicPointer>
template<typename T>
struct AtomicPtr
{
    QAtomicPointer<T> value;
    AtomicPtr(T t = nullptr) : value(t) {}
    T Get() const { return value.load(); }
    void Set(T s) { value.storeRelease(s); }
    AtomicPtr& operator=(T value) {
        Set(value);
        return *this;
    }
    operator T() const { return Get(); }
};

#include <QPointer>
// 不需要手动删除，QPointer 会自动处理 当对象被删除时自动置空
// 用于ws通信协议参考JSON-RPC 2.0 协议
// 请求模型
struct Session {
    qint64 id{ 0 };///< @brief 请求ID 可选
    int code{ 0 };///< @brief 执行的错误码,非0为执行异常 可选
    QPointer<QObject> socket;///< @brief 请求的socket 可选
    QString module{ "" }; ///< @brief 可选
    QString method;///< @brief 需要调用的函数,槽等 必须
    QJsonValue params;///< @brief  请求参数,如果是一个数组,则反射动态调用槽函数，可以为空;否则调用默认的带完整请求的槽函数 可选
    QJsonValue result;///< @brief 执行的结果 可选
    QString message;///< @brief 执行的消息 可选
    // QVariant context;///< @brief  上下文信息 可选 保留

    // 默认构造函数
    Session() = default; // 默认构造函数
    ~Session() {
        //if (socket) delete socket;
    }
    operator bool() const { return code == 0; }
    // 从 JSON 字符串解析为 Request
    Session(QJsonObject json) {
        if (json.contains("id")) id = json["id"].toInteger(0);
        if (json.contains("code")) code = json["code"].toInt(-1);
        if (json.contains("module")) module = json["module"].toString("");
        if (json.contains("method")) method = json["method"].toString("");
        if (json.contains("params")) params = json["params"];
        if (json.contains("result")) result = json["result"];
        if (json.contains("message")) message = json["message"].toString("");
        // if (json.contains("context")) context = json["context"].toVariant();
    }
    //使用请求体参数构造函数
    Session(const QString& module, const QString& method, const QJsonValue& params = QJsonValue()){
        this->module = module;this->method = method;this->params = params; this->id = NextId();
    }
    // 请求的Request发送
    static QString RequestString(const QString& module, const QString& method, const QJsonValue& params) {
        return JsonToString({ {"id", NextId()}, {"module", module}, {"method", method}, {"params", params} });
    }
    static QString RequestString(qint64 id, const QString& module, const QString& method, const QJsonValue& params) {
        return JsonToString({ {"id", id}, {"module", module}, {"method", method}, {"params", params} });
    }
    QString ErrorString(int errorCode, const QString& message) const {
        return JsonToString({ {"id", id}, {"code", errorCode},{"module", module}, {"method", method},{"params", params}, {"message", message} });
    }
    QString ResponseString(const QJsonValue& ExecutionResult,const QString& ExecutionMessage = QString()) const {
        return JsonToString({ {"id", id}, {"code",0},{"module", module}, { "method", method }, {"params", params},{"result", ExecutionResult}, {"message", ExecutionMessage} });
    }
    QString GetRequest() const {
        return JsonToString({ {"id", id}, {"module", module}, {"method", method}, {"params", params}, {"message", message} });
    }
    static qint64 NextId(){// 生成下一个唯一ID
        static QAtomicInteger<quint64> counter{0};
        static constexpr int kShiftBits = 20;  // 计数器位数
        static constexpr quint64 kMask = (1 << kShiftBits) - 1;// 计数器掩码(低20位为1)
        // static constexpr quint64 kEpoch = 1609459200000LL;//// 2021-01-01 00:00:00 作为基准时间
        const qint64 now = QDateTime::currentMSecsSinceEpoch();
        return (now << kShiftBits) | (++counter & kMask); // 时间戳左移20位,空出低20位给计数器 counter自增并取低20位,与时间戳组合形成唯一ID
    }
};

// 在头文件中注册类型 告诉Qt的元对象系统关于 Session 类的信息 1.可以在信号槽中使用 2.可以在 QVariant 中存储 3.可以在跨线程通信中使用
Q_DECLARE_METATYPE(Session)//
///用std::function定义处理器类型 QFunctionPointer 为Qt的函数指针类型无参数无类型返回值;不支持通过字符串动态查找函数。
///不支持跨线程调用（需要手动实现线程安全）。不支持信号槽机制。
///依赖于 Qt 的元对象系统，需要 Q_OBJECT 宏和 moc 预处理。性能开销较大，因为需要通过字符串查找函数。
using SessionHandler = std::function<void(Session&)>;

/*!
 * @var map gSession
 * @brief 全局会话默认处理方法,需要注册 使用宏定义自动生成映射
 *
 * @details 使用手动注册映射,无需查找元对象直接进行调用

 * @see SessionHandler
 */
#include <QMap>
inline QMap<QString, SessionHandler> gSession;
#define REGISTER_HANDLER(name) \
    gSession[#name] = &Class::name

inline void RegisterHandler(const QString& module, const QString& method, SessionHandler handler) {
    gSession[module + "." + method] = handler;
}
template<typename F>
void register_handler(const std::string& name, F&& handler) {//“万能引用”（Universal Reference）的语法。它可以绑定到左值或右值引用，具体取决于传入的参数类型。这使得函数可以接受各种类型的参数，包括临时对象和可移动对象。
    // gSession[name] = std::forward<F>(handler);// 模板函数，使用 std::forward 完美转发参数
    gSession.insert(QString::fromStdString(name), std::forward<F>(handler));
}

/*!
 * @brief 定义过滤器接口基类
 * @details 详细说明
 */
 // 全局过滤器映射
class SessionFilterable;
inline QList<SessionFilterable*> gSessionFilter;//使用函数映射不能直接应用到成员函数
class SessionFilterable {
public:
    virtual Result filter(Session& recv) = 0;// 纯虚函数，子类实现过滤逻辑
    SessionFilterable() {
        gSessionFilter.append(this);
    }
    virtual ~SessionFilterable() {
        gSessionFilter.removeOne(this);
    }
    // QMutexLocker locker(&gFilterMutex); // 线程安全
};

/*!
 * @brief 定义会话通信类型枚举
 * @details 详细说明
 */
#pragma pack(push, 1) // 1字节对齐
struct BinarySession {
    quint8 module;///< @brief 请求模块名 1字节 用于区分模块
    quint8 method;///< @brief 请求方法名 1字节 用于区分方法
    quint64 size;///< @brief 请求参数大小 8字节
    // 紧接着是数据 data[size]
};
#pragma pack(pop) // 恢复默认对齐

using BinarySessionHandler = std::function<void(const QByteArray&)>;
inline QMap<quint8, QMap<quint8, BinarySessionHandler>> gBinarySession;

// 在头文件中定义会话通信类型枚举
enum class SessionType : int {
    Unknown,
    Client,
    Server,
    Monitor,//监控
    Other
};

//适用插件类型使用
#include <QMetaEnum>
using TaskStateType = quint8;

/*任务状态枚举*/
#define TaskEnumName(name) TaskState_##name
enum TaskState : TaskStateType {
    TaskEnumName(Unknown),//未知
    TaskEnumName(Waiting),//等待
    TaskEnumName(PreStart),//准备开始
    TaskEnumName(Starting),//开始
    TaskEnumName(Started),//已开始
    TaskEnumName(Running),//运行中
    TaskEnumName(Finished),//已完成
    TaskEnumName(Paused),//暂停
    TaskEnumName(Resumed),//恢复
    TaskEnumName(Cancelled),//取消
    TaskEnumName(Timeout),//超时
    TaskEnumName(Failed),//失败
    TaskEnumName(Stopped),//停止
    TaskEnumName(Aborted),//中止
    TaskEnumName(Error)//错误
};

#include <QReadWriteLock>

/**
 * @brief 用于qt中QString和设备的const char*之间的转换
 * @note 主要解决QString的方法toUtf8() 返回临时 QByteArray，语句结束后内存立即释放
 * @note 其次是为了方便多线程高频读取,主线程修改的问题
 */
struct StringChar {
    StringChar(const char* __char = nullptr) {*this = __char;}
    StringChar(const QString& _string) {*this = _string;}
    // 重载 () 运算符实现无锁读取 （高频调用）
    QString operator()() const {
        return _char.loadRelaxed();
    }
    const char* getChar() const {
        return _char.loadRelaxed();
    }
    StringChar& operator=(const char* newChar) {
        if (!newChar) return *this; // nullptr check
        QWriteLocker locker(&_read_write_lock);
        _storage = QByteArray(newChar);// 深度拷贝
        _char.storeRelease(_storage.constData());
        return *this;
    }

    StringChar& operator=(const QString& newStr) {
        QWriteLocker locker(&_read_write_lock);
        _storage = newStr.toUtf8();
        _char.storeRelease(_storage.constData());
        return *this;
    }

    // 隐式转换为const char*
    operator const char* () const { return getChar(); }
private:
    QAtomicPointer<const char> _char = nullptr;
    QByteArray _storage;
    mutable QReadWriteLock _read_write_lock; // mutable允许const方法加锁
};

#endif
