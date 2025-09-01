#ifndef TROLLEYWIDGET_H
#define TROLLEYWIDGET_H
#include  <QPointer>
#include "ChildWidget.h"
#include "ui_TrolleyWidget.h"
// namespace Ui {
// class TrolleyWidget;
// }

class TrolleyWidget : public ChildWidget
{
    Q_OBJECT

public:
    Ui::TrolleyWidget *ui;
    explicit TrolleyWidget(MainWindow *parent = nullptr);
    ~TrolleyWidget();

    /*iController API 设备控制的接口方法*/
    void initialize() final;
    QString _module() const final;

    void test();
    void ShowMessage(const QString& msg);
    QJsonObject params;

    bool isRatedMileage{false}; //小车额定里程值,满足此条件,则结束任务
    int carRatedMileage;

    double lastTime;
    double lastMileag;
    void AddMileage(double time,double mileage, bool backward = false);// s秒 m米

    void handle_binary_message(const QByteArray &bytes);
public slots:
    // void onEnableChanged(bool enable=true) final;
    void initUi(const Session& session) final;
    void onConfigChanged(QJsonObject config) final;
    void onDeviceStateChanged(double state,QString message) final;

protected:
    QRadioButton* GetButtonDeviceManager() final;
    QWidget* GetWidgetDeviceManager() final;
    //实时采集情况监控界面
    QWidget* GetWidgetAcquisitionMonitor() final;
protected slots:
    void retranslate_ui() final; //更新显示语言
    void showEvent(QShowEvent *) override;;//打开窗口时执行
private slots:
    void on_pushButton_save_clicked();
    //设备串口选择
    void on_pushButton_scan_clicked();

    void on_pushButton_open_clicked();
    //控制启动
    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();
    //参数设置
    void on_comboBox_car_param_templates_activated(int index);

    void on_radioButton_Direction_Drive_clicked();

    void on_radioButton_Direction_Reverse_clicked();

    void on_horizontalScrollBar_car_travel_speed_valueChanged(int value);

    void on_spinBox_car_travel_speed_valueChanged(int arg1);

    void on_pushButton_set_car_speed_clicked();

    void on_comboBox_speed_multiplier_activated(int index);

    void on_radioButton_mileage_set_on_clicked();

    void on_radioButton_mileage_set_off_clicked();

private:
    void retranslate();// 对自定义组件（如图表） 更新显示,重新翻译
};

#endif // TROLLEYWIDGET_H
