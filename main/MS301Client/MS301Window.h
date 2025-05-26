#ifndef MS301WINDOW_H
#define MS301WINDOW_H

#include <QApplication>
#include <QTranslator>
inline QString g_language;
inline QTranslator g_translator;//qt的国际化

#include "ui_MS301Window.h"

class MS301Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit MS301Window(QWidget *parent = nullptr);
    FileInfoDetails currentProjectItem{};

    void AddProject(QJsonObject project);
    void UpdateProjects();

public slots:
    void GotoHomePage();
private slots:
    void on_actionReturnPageHome_triggered();

    void on_pushButton_project_hub_clicked();

    void on_pushButton_system_settings_clicked();

    void on_pushButton_device_management_clicked();

    void on_pushButton_parameter_templates_clicked();

    void on_pushButton_data_copy_clicked();

    void on_pushButton_project_start_clicked();

    void on_toolButton_alram_clicked();

    void on_toolButton_add_project_clicked();

private:
    Ui::MS301Window ui;
};

#endif // MS301WINDOW_H
