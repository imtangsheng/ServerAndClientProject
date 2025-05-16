#pragma once
#include <QObject>
// 用于Object执向的全局父类
//inline QObject gUserObject;

#include <QTranslator>
#include <QCoreApplication>

/**
 * @brief 用户服务管理类
 * 负责管理所有的用户服务,包含网络的启动,插件管理器类,任务执行等,几乎所有的主要控制类
 */
class UserServer : public QObject
{
    Q_OBJECT
public:
    QString module_;
    explicit UserServer(QObject* parent = nullptr);
    ~UserServer();
    /**
     * @brief 初始化用户服务
     * @param port 端口号<em>默认8080</em>
     */
    void initialize(quint16 port);
    /**
     * @brief 获取用户服务实例
     * @return 用户服务实例指针
     */
    //static UserServer* instance() {
    //    static UserServer self(&gUserObject);
    //    return &self;
    //}
    
    Q_INVOKABLE void onDeviceStateChanged(Session session);//不使用 const Session& Qt 的元对象系统（MOC）会将 Q_INVOKABLE 方法编译 都能被统一调用
public slots:
    void onLanguageChanged(QString language);
    void onAutoStartedClicked(bool checked);

    //设置注册表参数 配置
    void SetRegisterSettings(const Session& session);
    // 任务执行方法
    void acquisition_begins(const Session& session);
    void acquisition_end(const Session& session);

    /**
     * @brief 关闭用户服务
     */
    void shutdown();

private:
    QTranslator translator;
signals:
    void closed();
};
