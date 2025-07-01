#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H


#include  <QPointer>
#include "ChildWidget.h"
#include "ui_CameraWidget.h"
// namespace Ui {
// class CameraWidget;
// }


class CameraWidget :public ChildWidget
{
    Q_OBJECT
public:
    Ui::CameraWidget *ui;
    explicit CameraWidget(MainWindow *parent = nullptr);
    ~CameraWidget();
    /*iController API 设备控制的接口方法*/
    void initialize() final;
    QString _module() const final;

    // QJsonObject parameter; //统一的相机配置参数界面json对象
    // QJsonObject task; //执行任务的时候的参数
    // QJsonObject general;//通用配置参数
    void ShowMessage(const QString& msg);

    void handle_binary_message(const QByteArray &bytes);
public slots:
    void initUi(const Session& session) final;
    void onDeviceStateChanged(double state,QString message) final;
    void onConfigChanged(QJsonObject config);
    void onImageInfoChanged(const Session& session);
protected:
    QRadioButton* GetButtonDeviceManager() final;
    QWidget* GetWidgetDeviceManager() final;
    //实时采集情况监控界面
    QWidget* GetWidgetAcquisitionMonitor() final;
protected slots:
    void retranslate_ui() final; //更新显示语言
private slots:
    void on_comboBox_device_list_currentTextChanged(const QString &arg1);

    void on_pushButton_scan_clicked();

    void on_pushButton_task_key_reset_clicked();

    void on_comboBox_param_keys_currentTextChanged(const QString &arg1);

    void on_pushButton_param_setting_clicked();

    void on_pushButton_param_add_key_clicked();

    void on_pushButton_param_delete_key_clicked();

    void on_pushButton_param_server_save_clicked();

    void on_pushButton_trigger_clicked();

    void on_pushButton_param_server_get_clicked();

    void on_pushButton_open_clicked();

    void on_pushButton_start_clicked();

    void on_pushButton_image_format_set_clicked();

    void on_toolButton_image_clicked();

    void on_pushButton_serial_names_set_clicked();

private:

};
#endif // CAMERAWIDGET_H
