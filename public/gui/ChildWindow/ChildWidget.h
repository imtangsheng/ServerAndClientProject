#ifndef CHILDWIDGET_H
#define CHILDWIDGET_H
// 基础接口定义 使用组合模式,嵌入主界面的操作 对应插件的接口文件
class MainWindow;
#include <QRadioButton>
#include <QWidget>
class ChildWidget: public QWidget {
    Q_OBJECT
public:
    enum DeviceType {
        Trolley,
        Scanner,
        Camera,
        Other
    };
    Q_ENUM(DeviceType)
    ChildWidget(MainWindow *parent = nullptr);
    virtual ~ChildWidget() = default;

    DeviceType deviceType{Other};//用于设备特有的界面操作等
    virtual QString _module() const = 0;//记录当前设备模块名称 必要用于注册设备
    double state_{0};//记录当前设备状态值
    QString stateString;//监控显示信息
    QJsonObject config_;//设备参数 json格式
    QJsonObject parameter; //统一的置参数界面json对象
    QJsonObject task; //执行任务的时候的参数
    QJsonObject general;//通用配置参数
    // QJsonObject GetGeneral(){
    //     return config_.value("general").toObject();//返回的是临时对象
    // }

    virtual void initialize();
    bool isInitUi{false};

    /*同步更新任务配置相关,以及同步设置任务参数*/
    virtual Result SetTaskParameter(QJsonObject &data) = 0;
    virtual void UpdateTaskConfigSync(QJsonObject &content) = 0;

    void stop();//停止任务执行
public slots:
    virtual void onEnableChanged(bool enable=true);//设备模块是否激活,在线状态显示
    virtual void initUi(const Session& session) = 0;
    virtual void onConfigChanged(QJsonObject config) = 0;
    virtual void onDeviceStateChanged(double state,QString message){
        qDebug() <<"[#IWidget]"<<deviceType <<"state:" <<state << message;
        onEnableChanged(false);
    }//设备登录状态显示
protected:
    MainWindow* mainWindow{nullptr};  // 保存主窗口指针
    //设备管理界面
    virtual QRadioButton* GetButtonDeviceManager() = 0;
    virtual QWidget* GetWidgetDeviceManager() = 0;
    //实时采集情况监控界面
    virtual QWidget* GetWidgetAcquisitionMonitor() = 0;
protected slots:
    virtual void retranslate_ui() = 0; //更新显示语言
};


#endif // CHILDWIDGET_H
