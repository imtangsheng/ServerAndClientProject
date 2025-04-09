#ifndef IWIDGET_H
#define IWIDGET_H
// 基础接口定义 使用组合模式,嵌入主界面的操作
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
    IWidget(MainWindow *parent = nullptr);
    virtual ~IWidget() = default;
    virtual void initialize();

    DeviceType type{Other};
protected:
    MainWindow* mainWindow{nullptr};  // 保存主窗口指针
    //设备管理界面
    QRadioButton* buttonDeviceManager_{nullptr};
    QWidget* widgetDeviceManager_{nullptr};
    //实时采集情况监控界面
    QWidget* widgetAcquisitionMonitor_{nullptr};
    void setDeviceState(DeviceType type,bool offline = true);
protected slots:
    virtual void retranslate_ui() = 0; //更新显示语言
};


#endif // IWIDGET_H
