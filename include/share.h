/*!
 * @file Share共享库的声明文件
 * @brief 共享库的声明文件->cn
 * @date 2025-02
 */
#ifndef SHARE_H
#define SHARE_H
#include <QPromise>
#include <QFuture>
#include <QSettings>
#include "global.h"
#include "Logger.h"
#include <qcoreapplication.h>
 //定义快捷方式
#define gSouth south::Share::instance()
#define sigSouthSend south::Share::instance().sigSend

namespace south {

	class SHAREDLIB_EXPORT Share : public QObject
	{
		Q_OBJECT
	public:
		static Share& instance() {//使用引用,返回其静态变量,不进行拷贝数据
			static Share instance;
			return instance;
		}
		//目前支持的图像格式包括：bmp, jpeg, jpg, png, tiff, tif, gif, dat(纯图像数据)）
		const QStringList kImageFormat{ "jpeg", "jpg", "png","bmp","tiff", "tif", "gif", "dat" };
		QString appDirPath{ "../" };
		QSharedPointer<QSettings> RegisterSettings;//注册表设置
		void init(const QString& appPath, const QString& appName) {
			appDirPath = appPath;
			RegisterSettings.reset(new QSettings("South_Software", appName));
			getConfigSettings().reset(new QSettings(QString("%1/config/%2.ini").arg(appPath).arg(appName), QSettings::IniFormat));
		}
		// 注册处理器实例
		void registerHandler(const QString& module, QObject* handler) {
			qDebug() << "Registering handler for module:" << module << QThread::currentThread();
			handlers[module] = handler;//静态变量的生命周期与程序相同，无法在 Controller 销毁时释放资源
			//handler->setParent(this);//QObject::setParent: Cannot set parent, new parent is in a different thread
		}
		/**
		* @brief 使用宏定义自动生成映射
		* @param message 消息内容
		* @param client 需要发送的客户端
		*/
		Result invoke(QJsonObject json, QPointer<QObject> client) {
			Session session(json);
			if (!handlers.contains(session.module)) {
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
			QObject* targetObject = handlers[session.module];
			QByteArray utf8Data = session.method.toUtf8();//toUtf8()返回的临时QByteArray对象在语句执行完毕后被销毁，导致指向无效字符的指针。
			const char* method = utf8Data.constData();//method指针指向utf8Data对象,仍然有效。
			bool success{ false };// 调用方法目标模块的对象返回
			/// 1.使用可变参数模板调用 std::apply(func, std::tuple);需要确定参数个数,std::make_index_sequence<>常量类型故不使用
			/// 2.使用模板递归的方法,需要引用常量类型,不支持动态参数
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
		QFuture<Result> invokeAsync(const Session& session) {
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
						emit sigSend(session.toErrorString(-2, tr("调用失败")), session.socket);
						qWarning() << "Failed to invoke handler" << session.module;
						promise->addResult(Result(false, "调用失败"));// 成功时返回结果
					}
				}
				catch (const std::exception& e) {
					emit sigSend(session.toErrorString(-2, e.what()), session.socket);
					qWarning() << "Failed to invoke handler" << e.what();
					promise->addResult(Result(false, e.what()));// 成功时返回结果
				}
				// 完成 promise
				promise->finish();
				}, Qt::QueuedConnection);// 指定使用队列连接方式
			// 5. 立即返回future对象
			return future;
		}

		static QSharedPointer<QSettings>& getConfigSettings() {
			static QSharedPointer<QSettings> settings;
			return settings;
		}

		Result findFilePath(const QString& fileName, QString& validConfigPath);
		Result readJsonFile(const QString& filePath, QJsonObject& json);
		Result writeJsonFile(const QString& filePath, const QJsonObject& json);

	public slots:
		void onSend(const Result& result, const Session& session);
	protected:
		// 保护构造函数,只能继承使用
		explicit Share(QObject* parent = nullptr) : QObject(parent) {
			qDebug() << "Share::Share() 构造函数 - Current thread:" << QThread::currentThread();
			// 取消线程:确保对象移动到目标线程,构造函数本身的代码仍在原始线程（如主线程）中执行，只有对象的信号槽和事件处理会迁移到目标线程。线程亲和性(thread affinity)有严格限制
		}
	private:
		QMap<QString, QObject*> handlers;//需要手动管理
		~Share() {
			qDebug() << "Share::~Share() 析构函数 - Current thread:" << QThread::currentThread();
			handlers.clear();
		}
		Share(const Share&) = delete;
		Share& operator=(const Share&) = delete;
	signals:
		void sigSend(const QString& message, QObject* client = nullptr);//多线程的网络发送,需要使用信号连接到统一的线程中发送信息
	};

}//end namespace south
#endif // SOUTH_H
