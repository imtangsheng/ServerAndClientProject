/**
 * @file global.h
 * @brief 全局文件,常用的内联函数,函数返回类型,网络通信类型等数据结构类型进行定义
 * @author Tang
 * @date 2025-03-10
 */
#ifndef GLOBAL_H
#define GLOBAL_H
#include <QtCore/qglobal.h>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

#if defined(SHAREDLIB_API)
#define SHAREDLIB_EXPORT Q_DECL_EXPORT
#else
#define SHAREDLIB_EXPORT Q_DECL_IMPORT
#endif

inline static QJsonObject StringToJson(const QString& jsonString) {
	QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
	if (!jsonDoc.isNull() && jsonDoc.isObject()) {
		return jsonDoc.object();
	}
	else {
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
    QString message{""};
	Result(int i, const QString& msg = "") :code(i), message(msg) {}
    Result(bool s = true, const QString& msg = "") :message(msg) {
		if (s) {code = 0;}
	}//隐式构造函数调用
    static Result Success(const QString& msg = "") { return Result(true, msg); } 
    static Result Failure(const QString& msg = "") { return Result(false, msg); }
    operator bool() const { return code==0; }//重载了 bool 操作符，使其可以像之前的 bool 返回值一样使用例如：if (result)

};
// 声明结构体为元类型
//Q_DECLARE_METATYPE(Result)
// 注册 结构体为元类型 运行时环境初始化时调用  main 函数或者构造函数中
//qRegisterMetaType<Result>("Result");

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
	int id{ 0 };///< @brief 请求ID 可选
	int code{ 0 };///< @brief 执行的错误码,非0为执行异常 可选
	QPointer<QObject> socket;///< @brief 请求的socket 可选
	QString module{ "" }; ///< @brief 可选
	QString method;///< @brief 需要调用的函数,槽等 必须
	QJsonValue params;///< @brief  请求参数,如果是一个数组,则反射动态调用槽函数，可以为空;否则调用默认的带完整请求的槽函数 可选
	QJsonValue result;///< @brief 执行的结果 可选
	QString message;///< @brief 执行的消息 可选
	QVariant context;///< @brief  上下文信息 可选

	// 默认构造函数
	Session() = default; // 默认构造函数
	~Session() {
		//if (socket) delete socket;
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
		if (json.contains("message")) message = json["message"].toString("");
		if (json.contains("context")) context = json["context"].toVariant();
	}
	// 请求的Request发送
	static QString RequestString(int id, const QString& module, const QString& method, const QJsonValue& params) {
		return JsonToString({ {"id", id}, {"module", module}, {"method", method}, {"params", params} });
	}
	QString ErrorString(int errorCode, const QString& message) const {
		return JsonToString({ {"id", id}, {"code", errorCode},{"module", module}, {"method", method},{"params", params}, {"message", message} });
	}
	QString ResponseString(const QString& ExecutionMessage = QString()) const {
		return JsonToString({ {"id", id}, {"code",0},{"module", module}, { "method", method }, {"params", params},{"result", result}, {"message", ExecutionMessage} });
	}
    QString getRequest() const {
        return JsonToString({ {"id", id}, {"module", module}, {"method", method}, {"params", params}, {"message", message} });
    }
};
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

// 在头文件中定义会话通信类型枚举
enum class SessionType {
	Unknown,
	Client,
	Server,
	Monitor,//监控
	Other
};
#endif // GLOBAL_H
