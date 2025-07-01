#ifndef TROLLEYWIDGET_H
#define TROLLEYWIDGET_H

#include  <QPointer>
#include "ChildWidget.h"
#include "ui_TrolleyWidget.h"
// namespace Ui {
// class TrolleyWidget;
// }

class TrolleyWidget : public ChildWidget
{
    Q_OBJECT

public:
    Ui::TrolleyWidget *ui;
    explicit TrolleyWidget(MainWindow *parent = nullptr);
    ~TrolleyWidget();

    /*iController API 设备控制的接口方法*/
    void initialize() final;
    QString _module() const final;

    void test();
    void ShowMessage(const QString& msg);
    QJsonObject params;
public slots:
    void initUi(const Session& session) final;
    void onConfigChanged(QJsonObject config) final;
    void onDeviceStateChanged(double state,QString message) final;
protected:
    QRadioButton* GetButtonDeviceManager() final;
    QWidget* GetWidgetDeviceManager() final;
    //实时采集情况监控界面
    QWidget* GetWidgetAcquisitionMonitor() final;
    // void paintEvent(QPaintEvent *event) override;
protected slots:
    void retranslate_ui() final; //更新显示语言
    void showEvent(QShowEvent *) override;;//打开窗口时执行
private slots:
    void on_pushButton_set_speed_multiplier_clicked();

    void on_pushButton_save_clicked();

    void on_pushButton_scan_clicked();

    void on_pushButton_open_clicked();

private:
    void retranslate();// 对自定义组件（如图表） 更新显示,重新翻译
};

#endif // TROLLEYWIDGET_H
