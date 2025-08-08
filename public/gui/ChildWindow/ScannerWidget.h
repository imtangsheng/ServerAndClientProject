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

public:
    Ui::ScannerWidget *ui;
    explicit ScannerWidget(MainWindow *parent = nullptr);
    ~ScannerWidget();

    /*iController API 设备控制的接口方法*/
    void initialize() final;
    QString _module() const final;

    void ShowMessage(const QString& msg);
    QJsonObject params;

public slots:
    void initUi(const Session& session) final;
    void onConfigChanged(QJsonObject config) final;
    void onDeviceStateChanged(double state,QString message) final;
    void onWatcherFilesChanged(QJsonObject obj);
protected:
    QRadioButton* GetButtonDeviceManager() final;
    QWidget* GetWidgetDeviceManager() final;
    //实时采集情况监控界面
    QWidget* GetWidgetAcquisitionMonitor() final;
protected slots:
    void retranslate_ui() final; //更新显示语言
private slots:
    void on_comboBox_param_templates_activated(int index);

    void on_pushButton_power_on_clicked();

    void on_pushButton_power_off_clicked();

    void on_pushButton_diameter_height_measurement_clicked();

    void on_pushButton_task_management_clicked();

    void on_pushButton_start_clicked();

    void on_pushButton_connect_clicked();

    void on_radioButton_rate_1_clicked();

    void on_radioButton_rate_2_clicked();

    void on_radioButton_rate_4_clicked();

    void on_radioButton_rate_8_clicked();

    void on_pushButton_update_clicked();

    void on_spinBox_number_scan_lines_editingFinished();

    void on_spinBox_number_block_lines_editingFinished();

private:
    // void retranslate();
};

#endif // FAROWIDGET_H
