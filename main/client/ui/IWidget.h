#ifndef IWIDGET_H
#define IWIDGET_H
// 基础接口定义 使用组合模式,嵌入主界面的操作 对应插件的接口文件
class MainWindow;
#include <QRadioButton>
#include <QWidget>
class IWidget: public QWidget {
    Q_OBJECT
public:
    enum DeviceType {
        Trolley,
        Scanner,
        Camera,
        Other
    };
    Q_ENUM(DeviceType)
    IWidget(MainWindow *parent = nullptr);
    virtual ~IWidget() = default;

    DeviceType deviceType{Other};//用于设备特有的界面操作等
    virtual QString _module() const = 0;//记录当前设备模块名称 必要用于注册设备
    double state_{0};//记录当前设备状态值
    QString stateString;//监控显示信息
    QJsonObject config_;//设备参数 json格式

    virtual void initialize();
    bool isInitUi{false};

    Q_INVOKABLE void setDeviceState(bool offline=true);//设备登录状态显示
public slots:
    virtual void initUi(const Session& session) = 0;
    virtual void onDeviceStateChanged(double state,QString message){
        qDebug() <<"[#IWidget]"<<deviceType <<"state:" <<state << message;
        setDeviceState(false);
    };//设备登录状态显示
protected:
    MainWindow* mainWindow{nullptr};  // 保存主窗口指针
    //设备管理界面
    QRadioButton* buttonDeviceManager_{nullptr};
    QWidget* widgetDeviceManager_{nullptr};
    //实时采集情况监控界面
    QWidget* widgetAcquisitionMonitor_{nullptr};
protected slots:
    virtual void retranslate_ui() = 0; //更新显示语言
};


#endif // IWIDGET_H
