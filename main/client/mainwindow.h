#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include "ui_mainwindow.h"

// inline QObject*g_parent = new QObject();

#include <QMouseEvent>
#include <QEvent>
#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

// 读取系统主题设置
inline bool IsSystemDarkMode() {
    HKEY hKey;
    DWORD value = 1; // 默认浅色模式
    DWORD size = sizeof(DWORD);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, L"AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    return value == 0; // 0 表示深色模式，1 表示浅色模式
}

inline void SetDarkMode(HWND hwnd,BOOL darkMode = TRUE) {
    // BOOL darkMode = TRUE;
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
    if (FAILED(hr)) {
        qWarning("设置主题深色模式失败，错误代码: %ld", hr);
    }
}

#include <QTranslator>
inline QTranslator g_translator;//qt的国际化
// namespace Ui {
// class MainWindow;
// }

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    Ui::MainWindow *ui;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // QTranslator translator;

    // 修改函数签名，直接传值而不是引用
    Q_INVOKABLE void login_verify(double type,const QString& version);//设备登录验证程序
public slots:    
    void show_message(const QString& message, LogLevel level);
protected:
    // 重写 nativeEvent 以处理 Windows 原生消息
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
    bool eventFilter(QObject *obj, QEvent *event) override ;
protected slots:
    void onAacquisitionStartClicked();
private slots:
    void on_pushButton_Camera_clicked();

    void on_pushButton_language_switch_clicked();

    void on_actionEnglish_triggered();

    void on_actionChinese_triggered();

    void on_pushButton_devices_powered_off_clicked();

    void on_pushButton_test_clicked();

    void on_pushButton_AcquisitionBegins_clicked();

    void on_pushButton_AcquisitionEnd_clicked();

    void on_pushButton_AcquisitionCreate_clicked();

private:

signals:
    void languageChanged();

};

#endif // MAINWINDOW_H
