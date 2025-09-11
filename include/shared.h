/*!
 * @file Share共享库的声明文件
 * @brief 共享库的声明文件->cn
 * @date 2025-02
 */
#ifndef _SHARED_H_
#define _SHARED_H_
#include <QPromise>
#include <QFuture>
#include <QSettings>
#include <QTranslator>
#include "global.h"
//定义快捷方式
#define gShare share::Shared::instance()
#define gSigSent share::Shared::instance().sigSent
#define gSettings share::Shared::GetConfigSettings()


//定义模块名称 对非的插件类适用
constexpr auto sModuleScanner = "scanner";
constexpr auto sModuleCamera = "camera";
constexpr auto sModuleManager = "manager";
constexpr auto sModuleSerial = "serial";
constexpr auto sModuleOther = "other";
constexpr auto sModuleUser = "user";

/*@brief 定义注册表的键值*/
constexpr auto CAMERA_KEY_PORTNAME = "CameraPortName" ;
constexpr auto TASK_KEY_DATAPAHT = "TaskDataPath";

/*本地变量保存类型*/
static const QString zh_CN = "zh_CN";
static const QString en_US = "en_US";

namespace share {
    Q_NAMESPACE
        enum ModuleName {
        trolley,
        serial,
        scanner,
        camera,
        manager,
        other,
        user
    };
    Q_ENUM_NS(ModuleName)
//Share名称被 msvc使用,故使用Shared
class SHAREDLIB_EXPORT Shared : public QObject
{
    Q_OBJECT
public:
    static Shared& instance() {//使用引用,返回其静态变量,不进行拷贝数据
        static Shared instance;
        return instance;
    }
    //适用模块使用
    static QString GetModuleName(ModuleName name) {
        // 获取枚举值对应的字符串
        // 方法1：使用 QMetaEnum
        static QMetaEnum metaEnum = QMetaEnum::fromType<ModuleName>();
        return metaEnum.valueToKey(name);
        // 方法2：直接使用 QVariant
        //return QVariant::fromValue(name).toString();
    }

    static ModuleName GetModuleName(const QString& name) {
        // 从字符串转换为枚举值
        // 方法1：使用 QMetaEnum
        static QMetaEnum metaEnum = QMetaEnum::fromType<ModuleName>();
        return static_cast<ModuleName>(metaEnum.keyToValue(name.toStdString().c_str()));
        // 方法2：直接使用 QVariant
        //return QVariant(name).value<ModuleName>();
    }
    int sessiontype_{ 0 };//注册会话通话的类型,用于验证设备通信是否连接正确
    static QString GetVersion ();//项目版本信息 宏定义
    QString language;//记录当前显示语言

    QString appPath{ "../" };
    QJsonObject info;//程序应用信息
    QSharedPointer<QSettings> RegisterSettings;//注册表设置,使用 invokeMethod方法调用函数
    bool isCarDriving{ false };//判断小车的行驶状态

    QMap<ModuleName, BinarySessionHandler> handlerBinarySession;
    void awake(const QString& path, const QString& appName);
    // 注册处理器实例
    bool RegisterHandler(const QString& module, QObject* handler) {
        qDebug() << "服务模块注册:" << module << QThread::currentThread();
        handlers[module] = handler;//静态变量的生命周期与程序相同，无法在 Controller 销毁时释放资源
        //handler->setParent(this);//QObject::setParent: Cannot set parent, new parent is in a different thread
        return true;
    }
    QStringList GetHandlerList() {
        return handlers.keys();
    }
#pragma region InvokeMethod
    /**
		* @brief 使用宏定义自动生成映射
		* @param message 消息内容
		* @param client 需要发送的客户端
		*/
    Result invoke(QJsonObject json, QPointer<QObject> client) {
        Session session(json);
        if (!handlers.contains(session.module)) {
            qWarning() << "Module not found:" << session.module;
            return Result(false, session.Finished(-1, tr("设备模块未找到:%1").arg(session.module)));
        }
        session.socket = client; //Q_INVOKABLE 使方法可以通过信号与槽机制、或者通过 QMetaObject::invokeMethod 等方式动态调用
        return invoke(session);
    }
    Result invoke(const Session& session);
    QFuture<Result> invoke_async(const Session& session);
#pragma endregion

    static QSharedPointer<QSettings>& GetConfigSettings() {
        static QSharedPointer<QSettings> settings;
        return settings;
    }

    Result FindFilePath(const QString& fileName, QString& validConfigPath);
    inline void on_session(const QString& message, QObject* client = nullptr);//向连接端发送消息
public slots:
    void on_success(const QString& msg, const Session& session);//发送成功消息
    void on_send(const Result& result, const Session& session);//结果自动发送消息
protected:
    // 保护构造函数,只能继承使用
    explicit Shared(QObject* parent = nullptr) : QObject(parent) {
        qDebug() << tr("Share::Share() - 当前线程:") << QThread::currentThread();
        // 取消线程:确保对象移动到目标线程,构造函数本身的代码仍在原始线程（如主线程）中执行，只有对象的信号槽和事件处理会迁移到目标线程。线程亲和性(thread affinity)有严格限制
    }
private:
    QMap<QString, QObject*> handlers;//需要手动管理
    ~Shared() {
        qDebug() << "Share::~Share() 析构函数 - 当前线程:" << QThread::currentThread();
        handlers.clear();
    }
    Shared(const Shared&) = delete;
    Shared& operator=(const Shared&) = delete;
signals:
    void sigSent(const QString& message, QObject* client = nullptr);//多线程的网络发送,需要使用信号连接到统一的线程中发送信息
    void sigSentBinary(const QByteArray& message, QObject* client = nullptr);//发送二进制数据
    void signal_set_window_title(const QString& title);
    void signal_language_changed(const QString& language); //模块接收语言改变的信号
    void signal_translator_load(QTranslator& translator, bool isLoad);//模块发出翻译器加载的信号
};


}//end namespace share

//向所有ws的客户端推送消息
inline void PushClients(const QString& method, const QJsonValue& params, const QString& module) {
    emit gSigSent(Session::RequestString(module, method, params));
}
//向指定的ws的客户端发送消息
inline void PushSessionResponse(const Session& session,const qint8& code, const QJsonValue& result) {
    if (Result(code))
        emit gSigSent(session.ResponseSuccess(result), session.socket);
    else
        emit gSigSent(session.Finished(code,result.toString()) , session.socket);
}

#endif
