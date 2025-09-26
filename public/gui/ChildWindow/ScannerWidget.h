#ifndef FAROWIDGET_H
#define FAROWIDGET_H

#include  <QPointer>
#include "ChildWidget.h"
// namespace Ui {
// class ScannerWidget;
// }
#include "ui_ScannerWidget.h"

class ScannerWidget : public ChildWidget
{
    Q_OBJECT
    enum ScannerState{
        ScanConnected,
        ScanStart,
        ScanPause,
        ScanStop
    };
public:
    Ui::ScannerWidget *ui;
    explicit ScannerWidget(MainWindow *parent = nullptr);
    ~ScannerWidget();

    /*iController API 设备控制的接口方法*/
    void initialize() final;
    QString _module() const final;
    Result SetTaskParameter(QJsonObject &data) final;
    void UpdateTaskConfigSync(QJsonObject &content) final;
    void ShowMessage(const QString& msg);
    QJsonObject params;
    void ScanPowerSwitch(bool open);//控制扫描上电开关
public slots:
    void onConnectionChanged(bool enable=true) final;
    void onUpdateUi(const QJsonObject& obj) final;
    void onConfigChanged(QJsonObject config) final;
    void onDeviceStateChanged(double state) final;
    void onWatcherFilesChanged(QJsonObject obj);
protected:
    QRadioButton* GetButtonDeviceManager() final;
    QWidget* GetWidgetDeviceManager() final;
    //实时采集情况监控界面
    QWidget* GetWidgetAcquisitionMonitor() final;
protected slots:
    void retranslate_ui() final; //更新显示语言
private slots:
    void on_pushButton_connect_clicked();

    void on_pushButton_update_clicked();
    //全局类参数
    void on_pushButton_serial_number_update_clicked();

    void on_comboBox_param_templates_activated(int index);

    void on_pushButton_power_switch_clicked();

    void on_pushButton_power_off_clicked();

    void on_pushButton_diameter_height_measurement_clicked();

    void on_pushButton_start_clicked();

    void on_spinBox_number_scan_lines_editingFinished();

    void on_spinBox_number_block_lines_editingFinished();

    void on_radioButton_rate_1_clicked();

    void on_radioButton_rate_2_clicked();

    void on_radioButton_rate_4_clicked();

    void on_radioButton_rate_8_clicked();

    void on_radioButton_quality_1_clicked();

    void on_radioButton_quality_2_clicked();

    void on_radioButton_quality_3_clicked();

    void on_radioButton_quality_4_clicked();

    void on_radioButton_quality_6_clicked();

    void on_radioButton_quality_8_clicked();

    void on_radioButton_resolution_1_clicked();

    void on_radioButton_resolution_2_clicked();

    void on_radioButton_resolution_4_clicked();

    void on_radioButton_resolution_5_clicked();

    void on_radioButton_resolution_8_clicked();

    void on_radioButton_resolution_10_clicked();

    void on_radioButton_resolution_16_clicked();

    void on_radioButton_resolution_20_clicked();

    void on_radioButton_resolution_32_clicked();

    void on_pushButton_ScanStart_clicked();

    void on_pushButton_ScanRecord_clicked();

    void on_pushButton_ScanPause_clicked();

    void on_pushButton_ScanStop_clicked();

    void on_pushButton_ScanSetParameter_clicked();



private:
    // void retranslate();
    bool isPowerSupply{false};//是否处于供电状态
};

inline QPointer<ScannerWidget>gScanner;
#endif // FAROWIDGET_H
