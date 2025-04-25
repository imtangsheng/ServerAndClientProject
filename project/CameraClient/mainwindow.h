#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QTranslator>
inline QString g_language;
inline QTranslator g_translator;//qt的国际化

#include <QString>

class WasmSettings {
public:
    static void setValue(const QString &key, const QString &value);
    static QString getValue(const QString &key, const QString &defaultValue = QString());
    static void remove(const QString &key);
    static void clear();
};

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
