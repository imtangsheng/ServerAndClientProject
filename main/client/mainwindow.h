#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QMouseEvent>
#include <QEvent>
#include <QTranslator>
inline QString g_language;
inline QTranslator g_translator;//qt的国际化
#include <QSequentialAnimationGroup>
#include "ui_mainwindow.h"
// namespace Ui {
// class MainWindow;
// }
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString module_ = south::ShareLib::GetModuleName(south::ModuleName::user);
    // gSouth.RegisterHandler(module_,&window);
    Ui::MainWindow *ui;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    // QTranslator translator;

    // 修改函数签名，直接传值而不是引用
    Q_INVOKABLE void login_verify(double type,const QString& version);//设备登录验证程序
public slots:
    void show_log_message(QString message, double level);

protected:
    // 重写 nativeEvent 以处理 Windows 原生消息
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;//qt5使用loog
    bool eventFilter(QObject *obj, QEvent *event) override ;
    void closeEvent(QCloseEvent *event) override;
protected slots:
    void onAacquisitionStartClicked();
    void onUserDocumentClicked();
    void onDeviceManagerClicked();
    void onDataManagerClicked();

    void onStateChanged(QAbstractSocket::SocketState state);
    // 网络连接状态:显示信息 是否在Connecting中 已连接Connected
    void onNetworkStateShow(const QString& msg,const bool& isConnecting = false,const bool& isConnected = false);
    void show_message(const QString& message, LogLevel level);
private slots:

    void on_pushButton_language_switch_clicked();

    void on_action_LanguageEnglish_triggered();

    void on_action_LanguageChinese_triggered();

    void on_toolButton_devices_powered_off_clicked();

    void on_pushButton_test_clicked();

    void on_pushButton_AcquisitionBegins_clicked();

    void on_pushButton_AcquisitionEnd_clicked();

    void on_pushButton_AcquisitionCreate_clicked();

    void on_pushButton_Network_State_clicked();

    void on_toolButton_Robot_clicked();

    void on_checkBox_AutoStart_clicked(bool checked);

private:
    void retranslate();//更新文本翻译

    QSequentialAnimationGroup* animationGroup;//动画效果
signals:
    void languageChanged();

};

#endif // MAINWINDOW_H
