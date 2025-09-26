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
    void initialize(quint16 port_ws=8080, quint16 port_http=80);
    /**
     * @brief 获取用户服务实例
     * @return 用户服务实例指针
     */
    //static UserServer* instance() {
    //    static UserServer self(&gUserObject);
    //    return &self;
    //}
    Q_INVOKABLE void onTest();
    //无论是否使用 const Session& ,Qt 的元对象系统（MOC）会将 Q_INVOKABLE 方法编译 都能被统一调用   
    Q_INVOKABLE void onDeviceStateChanged(const Session &session);
    Q_INVOKABLE void SetRealtimeParsing(Session session);
public slots:
    void onLanguageChanged(QString language);
    void onAutoStartedClicked(const Session& session);
    void onCarWarningClicked(const Session& session);
    void onLogLevelChanged(const Session& session);
    //设置注册表参数 配置
    void SetRegisterSettings(const Session& session);
    void GetAvailableSerialPorts(const Session& session);
    // 任务执行方法
    void GetTaskData(const Session& session);// 获取所有的任务数据信息
    void AddNewProject(const Session& session);// 添加新的项目
    void DeleteProject(const Session& session);
    void AddCurrentTask(const Session& session);//将任务添加并设置为当前任务。
    void DeleteTask(const Session& session); //删除任务
    void onStart(const Session& session, bool isContinue = false);
    void onStop(const Session& session, bool isContinue = false);

    /**
     * @brief 关闭用户服务
     */
    void shutdown(const Session& session);

private:
    QTranslator translator;
signals:
};
