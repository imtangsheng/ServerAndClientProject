#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QTranslator>
#include <QStyleFactory>
inline QString g_language;
inline QTranslator g_translator;//qt的国际化

#include "ui_MainWindow.h"
#include"button/ProjectItemCheckBox.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    FileInfoDetails currentProjectItem{};
    QList<ProjectItemCheckBox*> projectItemWidget;//用于显示项目,排序和删除的操作
    void AddProjectWidget(FileInfoDetails project);
    void UpdateLayoutProjectWidget();
    bool UpdateProjects();
    void SetLogLevel(LogLevel level);

public slots:
    void GotoHomePage();
    void onEnterProjectClicked(const FileInfoDetails& project);
    void onShowMessage(const QString &message, LogLevel level);

protected slots:
    void onConnectSocket();
    void onDisconnectSocket();
    void onSocketError(QAbstractSocket::SocketError error);    // 新增错误处理槽
protected:
    void changeEvent(QEvent *event) override;

private slots:
    void on_action_goto_home_triggered();
    void on_action_start_triggered();
    void on_action_stop_triggered();
    //首页界面
    void on_pushButton_project_hub_clicked();

    void on_pushButton_system_settings_clicked();

    void on_pushButton_device_management_clicked();

    void on_pushButton_parameter_templates_clicked();

    void on_pushButton_data_copy_clicked();

    void on_pushButton_project_start_clicked();

    void on_toolButton_alarm_clicked();

    void on_pushButton_shutdown_clicked();

    //系统设置页面
    void on_radioButton_language_en_US_clicked();

    void on_radioButton_language_en_US_toggled(bool checked);

    void on_pushButton_network_connect_clicked();

    void on_radioButton_startup_auto_off_clicked();

    void on_radioButton_startup_auto_off_toggled(bool checked);

    void on_radioButton_startup_fullscreen_on_clicked();

    void on_radioButton_startup_fullscreen_off_clicked();

    void on_radioButton_theme_light_clicked();

    void on_radioButton_theme_light_toggled(bool checked);

    void on_radioButton_car_warning_off_clicked();

    void on_radioButton_car_warning_off_toggled(bool checked);

    void on_radioButton_log_debug_clicked();

    void on_radioButton_log_info_clicked();

    void on_radioButton_log_warning_clicked();

    void on_radioButton_log_error_clicked();

    void on_pushButton_about_clicked();

    //项目页面
    void on_action_create_project_triggered();

    void on_checkBox_project_sort_clicked(bool checked);

    void on_dateTimeEdit_project_begin_dateTimeChanged(const QDateTime &dateTime);

    void on_pushButton_project_search_ok_clicked();

    void on_pushButton_project_delete_ok_clicked();

    void on_pushButton_project_delete_cancel_clicked();

    //任务页第一页
    void on_comboBox_city_activated(int index);

    void on_comboBox_task_param_line_name_activated(int index);

    void on_pushButton_task_param_content_tunnel_diameter_sub_1_clicked();

    void on_pushButton_task_param_content_tunnel_diameter_add_1_clicked();

    void on_pushButton_task_param_starting_ring_sub_50_clicked();

    void on_pushButton_task_param_starting_ring_add_50_clicked();

    void on_pushButton_task_param_start_mileage_sub_50_clicked();

    void on_pushButton_task_param_start_mileage_add_50_clicked();

    void on_pushButton_task_param_segment_width_sub_1_clicked();

    void on_pushButton_task_param_segment_width_add_1_clicked();

    void on_pushButton_task_param_first_page_next_step_clicked();

    void on_pushButton_tsak_param_first_page_cancel_clicked();
    //任务参数设置第二页
    void on_comboBox_parameter_templates_activated(int index);

    void on_pushButton_task_param_last_page_previous_step_clicked();

    void on_pushButton_task_param_last_page_next_step_prestart_clicked();

    void on_pushButton_task_param_last_page_cancel_clicked();
    //任务参数全局 响应变化
    void on_radioButton_car_obstacle_avoidance_enabled_clicked();

    void on_radioButton_car_obstacle_avoidance_close_clicked();

    void on_radioButton_realtime_parsing_enabled_clicked();

    void on_radioButton_realtime_parsing_off_clicked();

    void on_radioButton_car_forward_clicked();

    void on_radioButton_car_backward_clicked();

    void on_radioButton_camera_format_jpg_clicked();

    void on_radioButton_camera_format_jpeg_clicked();

    void on_radioButton_camera_format_png_clicked();

    void on_radioButton_camera_format_bmp_clicked();

    void on_radioButton_camera_format_raw_clicked();

    void on_pushButton_test_clicked();


private:
    friend class ChildWidget;
    Ui::MainWindow ui;
    void _retranslate();//更新文本翻译

    QJsonObject citySubwayInfo; //城市地铁线路站点信息
    void UpdateCitySubwayInfo(const QString& dirPathCity);
    void UpdateCameraFormat(const QString &format);
    void SetCameraFormat(const QString &format);
    QJsonArray parameterTemplatesInfo; //参数模板数组信息
    void UpdateLayoutParamTemplate();
    void CurrentParamTemplateChanged(int id);

signals:
    void languageChanged();
};

#endif
