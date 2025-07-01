#ifndef FAROWIDGET_H
#define FAROWIDGET_H

#include  <QPointer>
#include "ChildWidget.h"
// namespace Ui {
// class ScannerWidget;
// }
#include "ui_ScannerWidget.h"

class ScannerWidget : public ChildWidget
{
    Q_OBJECT

public:
    Ui::ScannerWidget *ui;
    explicit ScannerWidget(MainWindow *parent = nullptr);
    ~ScannerWidget();

    /*iController API 设备控制的接口方法*/
    void initialize() final;
    QString _module() const final;

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
protected slots:
    void retranslate_ui() final; //更新显示语言
private:
    // void retranslate();
};

#endif // FAROWIDGET_H
