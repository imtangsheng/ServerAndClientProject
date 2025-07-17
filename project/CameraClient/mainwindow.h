#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QTranslator>
inline QString g_language;
inline QTranslator g_translator;//qt的国际化
#include <QString>

#if defined(Q_OS_WINDOWS)
#include "public/utils/windows_utils.h"
#elif defined(Q_OS_WASM) //__EMSCRIPTEN __
#include "public/utils/wasm_utils.h"

#endif

#include <QMainWindow>
#include "MainControl.h"

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
    QString module_ = share::ShareLib::GetModuleName(share::ModuleName::user);
    // 修改函数签名，直接传值而不是引用
    Q_INVOKABLE void login_verify(double type,const QString& version);//设备登录验证程序
protected slots:
    void onStateChanged(QAbstractSocket::SocketState state);
    // 网络连接状态:显示信息 是否在Connecting中 已连接Connected
    void onNetworkStateShow(const QString& msg,const bool& isConnecting = false,const bool& isConnected = false);
    void show_message(const QString& message, LogLevel level=LogLevel::Debug);
private slots:
    void on_pushButton_language_switch_clicked();

    void on_pushButton_test_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
