#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include  <QPointer>
#include "IWidget.h"
#include "ui_CameraWidget.h"
// namespace Ui {
// class CameraWidget;
// }

#include "iController.h"
class CameraWidget :public IWidget,public iController
{
    Q_OBJECT
public:
    QString module{"camera"};
    Ui::CameraWidget *ui;
    explicit CameraWidget(MainWindow *parent = nullptr, const QString& module = "camera");
    ~CameraWidget();
    /*iController API 设备控制的接口方法*/
    void initialize() final;
    //interface IControllerSDK wrapper method implementations
    void  prepare() final;
    void start() final;
    void stop() final;

    void ShowMessage(const QString& msg);
    QJsonObject cameraParamsJson;
public slots:

protected slots:
    void retranslate_ui() final; //更新显示语言
private slots:
    void on_pushButton_test_clicked();

    void on_pushButton_scan_clicked();

    void on_pushButton_open_clicked();

    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();

    void on_pushButton_showProperty_clicked();

    void on_pushButton_param_setting_clicked();

    void on_pushButton_param_add_key_clicked();

    void on_pushButton_param_delete_key_clicked();

    void on_pushButton_param_server_save_clicked();

    void on_comboBox_param_keys_currentTextChanged(const QString &arg1);

    void on_pushButton_trigger_clicked();

    void on_pushButton_frame_update_clicked();

private:

};
extern QPointer<CameraWidget>gCamera;
#endif // CAMERAWIDGET_H
