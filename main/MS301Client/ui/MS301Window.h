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

// public slots:
    void GotoHomePage();
private slots:
    void on_toolButton_logo_clicked();

    void on_pushButton_project_hub_clicked();

private:
    Ui::MS301Window ui;
};

#endif // MS301WINDOW_H
