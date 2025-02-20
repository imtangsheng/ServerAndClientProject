#pragma once
// WSHandlerManager.h
#include <QObject>
#include <QMap>
#include <qDebug>
#include <QJsonObject>
#include <QPromise>
#include <QFuture>
#include <QUnhandledException>
#include <QException>
#include <QString>
#include <QSharedPointer>
#include <QWebSocket>
#include <QMetaMethod>
#include <QThread>

#include "SouthGlobal.h"

#define gWSMHandler WSHandlerManager::instance()
// 使用 QSharedPointer 管理 QWebSocket
//using WebSocketPtr = QSharedPointer<QWebSocket>;
using WebSocketPtr = QWebSocket*;
// 用于ws通信协议参考JSON-RPC 2.0 协议
// 请求模型
struct Request {
	int id{ 0 };// 请求ID
	QString module{ "" }; // 模块名称
	QString method; // 方法名称
	QJsonValue params; // 请求参数,默认是一个数组，可以为空。
	QVariant context; // 上下文信息（可选，用于存储额外的数据）
	WebSocketPtr socket{ nullptr };// 请求的socket
	// 默认构造函数
	Request() = default; // 默认构造函数
	~Request() = default;
	// 从 JSON 字符串构造
	Request(const QString& jsonString) {
		*this = fromJsonString(jsonString);
	}
	// 带参数的构造函数
	Request(int id, const QString& module, const QString& method, const QJsonValue& params, const QVariant& context = QVariant())
		: id(id), module(module), method(method), params(params), context(context) {}
	// 将 Request 转换为 JSON 字符串
	QString toJsonString() const {
		QJsonObject json;
		json["id"] = id; json["module"] = module; json["method"] = method; json["params"] = params;
		if (context.isValid()) {
			json["context"] = QJsonValue::fromVariant(context);
		}
		return QJsonDocument(json).toJson(QJsonDocument::Compact);
	}
	// 从 JSON 字符串解析为 Request
	Request(QJsonObject json) {
		*this = Request(json["id"].toInt(0), json["module"].toString(""),
			json["method"].toString(), json["params"],
			json.contains("context") ? json["context"].toVariant() : QVariant()
		);
	}
	// 从 JSON 字符串解析为 Request
	static Request fromJsonString(const QString& jsonString) {
		QJsonObject json = stringToJson(jsonString);
		return Request(json);
	}
	static QString toRequestString(int id, const QString& module, const QString& method, const QJsonValue& params) {
		return jsonToString({ {"id", id}, {"module", module}, {"method", method}, {"params", params} });
	}
};

//用于ws通信协议 响应模型
struct Response
{
	int id{ 0 };
	int code{ 0 };//执行的错误码,非0为执行异常
	QJsonValue result;
	QString message;
	Response() = default;
	Response(int id, int code, QJsonValue result, QString message)
		: id(id), code(code), result(result), message(message) {}
	static QString toString(int id, int code = 0, const QJsonValue& result = QJsonValue(), const QString& message = QString()) {
		return jsonToString({ {"id", id},{"code",code}, { "result", result }, {"message", message} });
	}
	static QString toErrorString(int id, int code, const QString& message) {
		return jsonToString({ {"id", id}, {"code", code},{"message", message} });
	}
	// 从 JSON 字符串解析为 Response
	static Response fromJsonString(const QString& jsonString) {
		QJsonObject json = stringToJson(jsonString);
		return Response(json["id"].toInt(0), json["code"].toInt(-1),
			json["result"], json["message"].toString());
	}
	operator bool() const { return code == 0; }
};

/**
 * @brief 使用std::function定义处理器类型 QFunctionPointer 为Qt的函数指针类型无参数无类型返回值
 * 不支持通过字符串动态查找函数。不支持跨线程调用（需要手动实现线程安全）。不支持信号槽机制。
 *
 * @brief QMetaObject::invokeMethod
 * 依赖于 Qt 的元对象系统，需要 Q_OBJECT 宏和 moc 预处理。性能开销较大，因为需要通过字符串查找函数。
 */
using SessionHandler = std::function<void(Request&)>;

/**
* @brief 使用宏定义自动生成映射
*/
inline QMap<QString, SessionHandler> gSessionMap;
#define REGISTER_HANDLER(name) \
    gSessionMap[#name] = &Class::name

inline void gRegisterHandler(const QString& module, const QString& method, SessionHandler handler) {
	gSessionMap[module + "." + method] = handler;
}
template<typename F>
void register_handler(const std::string& name, F&& handler) {
	gSessionMap[name] = std::forward<F>(handler);// 模板函数，使用 std::forward 完美转发参数
}


/**
 * @brief 如果直接调用 WSHandlerManager 的非信号槽方法（如 public 方法），这些方法会在调用者线程中执行，不会自动切换到目标线程。
 * 信号槽调用方法：如果通过信号槽机制调用 WSHandlerManager 的槽函数，槽函数会在目标线程（workerThread）中执行，前提是：对象已成功移动到 workerThread。
 */
class WSHandlerManager : public QObject
{
	Q_OBJECT
public:
	static WSHandlerManager& instance() {
		static WSHandlerManager instance;
		return instance;
	}
	template<std::size_t... I>
	bool invokeMethodHelper(QObject* targetObject,
		const QString& method,
		QGenericArgument* argv,
		std::index_sequence<I...>)
	{
		return QMetaObject::invokeMethod(
			targetObject,
			method.toUtf8().constData(),
			(I < sizeof...(I) ? argv[I] : QGenericArgument())...
		);
	}

	/**
	* @brief 使用宏定义自动生成映射
	* @param message 消息内容
	* @param client 需要发送的客户端
	*/
	Result invoke(QJsonObject json, WebSocketPtr client) {
		Request session(json);
		if (!m_handlers.contains(session.module)) {
			qWarning() << "Module not found:" << session.module;
			return Result(false, Response::toErrorString(session.id, -1, "设备模块未找到:" + session.module));
		}
		session.socket = client; //Q_INVOKABLE 使方法可以通过信号与槽机制、或者通过 QMetaObject::invokeMethod 等方式动态调用
		return invoke(session);
	}
	Result invoke(const Request& session) {
		// 获取目标模块的对象
		QObject* targetObject = m_handlers[session.module];
		bool success{ false };// 调用方法目标模块的对象返回
		if (session.params.isArray()) {// 判断是否为数组
			QJsonArray array = session.params.toArray();
			int paramCount = array.size();
			if (paramCount > 10) { // QMetaObject::invokeMethod 最多支持 10 个参数
				qWarning() << "Too many parameters for invokeMethod!";
				return Result(false, Response::toErrorString(session.id, -1, "Too many parameters"));
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
					return Result(false, Response::toErrorString(session.id, -1, "Unsupported parameter type"));
				}
			}
			// 使用可变参数模板调用 std::apply(func, std::tuple);需要确定参数个数,std::make_index_sequence<>常量类型故不使用
			switch (paramCount) {
			case 0:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData());
				break;
			case 1:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0]);
				break;
			case 2:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1]);
				break;
			case 3:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1], argv[2]);
				break;
			case 4:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1], argv[2], argv[3]);
				break;
			case 5:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1], argv[2], argv[3], argv[4]);
				break;
			case 6:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]);
				break;
			case 7:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
				break;
			case 8:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]);
				break;
			case 9:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]);
				break;
			case 10:
				success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]);
				break;
			default:
				qFatal() << "Unsupported number of arguments!";
				break;
			}
			delete[] argv;
		}
		else
		{
			// 默认使用请求对象作为参数 isNull()是否无效或者未定义值 其他参数调用默认 备注:空的""为字符串类型,会判断为有效值
			success = QMetaObject::invokeMethod(targetObject, session.method.toUtf8().constData(), Q_ARG(const Request&, session));
		}
		if (!success)//如果未找到调用的方法,返回失败,支持函数重载
		{
			return Result(false, Response::toErrorString(session.id, -1, tr("方法调用失败,请检测模块%1的方法:%2使用的参数是否正确").arg(session.module).arg(session.method)));
		}
		return true;
	}

	// 注册处理器实例
	void registerHandler(const QString& module, QObject* handler) {
		qDebug() << "Registering handler for module:" << module << QThread::currentThread();
		m_handlers[module] = handler;
		//handler->setParent(this);//QObject::setParent: Cannot set parent, new parent is in a different thread
	}
	// 异步调用并获取结果
	/** 调用异步函数
		// 调用异步方法
		QFuture<QJsonObject> future = invokeHandlerAsync("module1", "method1", data);
		// 方式1: 使用 then() 处理结果
		future.then([](const QJsonObject& result) {
			// 处理成功的结果
		}).onFailed([](const QException& error) {
			// 处理错误
		});
		// 方式2: 等待结果
		try {
			QJsonObject result = future.result();
			// 处理结果
		} catch (const QException& e) {
			// 处理错误
		}
	**/
	QFuture<Result> invokeHandlerAsync(const Request& session) {
		// 1. 创建Promise和Future
		// 使用 std::make_shared 来存储 promise
		// 使用 QSharedPointer 创建 promise
		auto promise = QSharedPointer<QPromise<Result>>(new QPromise<Result>());
		QFuture<Result> future = promise->future();
		// 2. 使用QMetaObject::invokeMethod在事件循环中执行操作
		QMetaObject::invokeMethod(this, [this, session, promise]() mutable {
			try {
				// 3. 执行实际操作
				Result success = invoke(session);
				// 4. 设置结果
				if (success) {
					promise->addResult(Result(true, "成功"));// 成功时返回结果
				}
				else {
					emit sigSend(session.socket, Response::toErrorString(session.id, 2, tr("调用失败")));
					qWarning() << "Failed to invoke handler" << session.module;
					promise->addResult(Result(false, "调用失败"));// 成功时返回结果
				}
			}
			catch (const std::exception& e) {
				emit sigSend(session.socket, Response::toErrorString(session.id, 2, e.what()));
				qWarning() << "Failed to invoke handler" << e.what();
				promise->addResult(Result(false, e.what()));// 成功时返回结果
			}
			// 完成 promise
			promise->finish();
			}, Qt::QueuedConnection);// 指定使用队列连接方式
		// 5. 立即返回future对象
		return future;
	}
private:
	QMap<QString, QObject*> m_handlers;
	QThread* workerThread{ nullptr };
	WSHandlerManager() {
		qDebug() << "WSHandlerManager:" << QThread::currentThread();
		this->setObjectName("WSHandlerManager");
		initialize();
	}
	void initialize() {
		workerThread = new QThread();
		workerThread->setObjectName("WSHandlerManagerThread");
		// 确保对象移动到目标线程,构造函数本身的代码仍在原始线程（如主线程）中执行，只有对象的信号槽和事件处理会迁移到目标线程。
		this->moveToThread(workerThread);
		workerThread->start();  // 先启动线程
	}
	~WSHandlerManager() {
		workerThread->quit();
		workerThread->wait();
		delete workerThread;
	}
	WSHandlerManager(const WSHandlerManager&) = delete;
	WSHandlerManager& operator=(const WSHandlerManager&) = delete;

signals:
	void sigSend(WebSocketPtr client, const QString& message);//多线程的网络发送,需要使用信号连接到统一的线程中发送信息
};


// WSHandlerBase.h
class WSHandlerBase : public QObject
{
	Q_OBJECT
public:
	explicit WSHandlerBase(const QString& module, QObject* parent = nullptr)
		: QObject(parent), m_module(module) {// 创建工作线程 对象有父对象时不能移动到其他线程。Qt的父子对象必须在同一个线程中。
		qDebug() << "WShandlerBase" << module << QThread::currentThread();
		// 自动注册到管理器
		WSHandlerManager::instance().registerHandler(module, this);
		initialize();
	}
	virtual ~WSHandlerBase() {
	}
	virtual void initialize() {
		// 派生类可以重写此方法进行线程特定的初始化
	}
protected:
	QString m_module;
};

// CRTP方式
template<typename T>
class BaseCRTP {
public:
	// 公共接口方法
	void interface() {// 调用基类接口，转发到派生类implementation
		qDebug() << "调用接口方法:BaseCRTP\n";
		static_cast<T*>(this)->implementation();
	}
	// 基类的默认实现
	void implementation() {
		qDebug() << "Base的默认实现";
	}
};

class DerivedCRTP : public BaseCRTP<DerivedCRTP> {
public:
	void implementation() {// 同名覆盖
		qDebug() << "DerivedCRTP的实现\n";
	}
};