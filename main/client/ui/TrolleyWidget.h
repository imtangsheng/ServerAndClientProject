#ifndef TROLLEYWIDGET_H
#define TROLLEYWIDGET_H

#include  <QPointer>
#include "IWidget.h"
#include "iController.h"

#include "ui_TrolleyWidget.h"
// namespace Ui {
// class TrolleyWidget;
// }

class TrolleyWidget : public IWidget,public iController
{
    Q_OBJECT

public:
    Ui::TrolleyWidget *ui;
    explicit TrolleyWidget(MainWindow *parent = nullptr);
    ~TrolleyWidget();

    /*iController API 设备控制的接口方法*/
    void initialize() final;
    //interface IControllerSDK wrapper method implementations
    void  prepare() final;
    void start() final;
    void stop() final;

    void ShowMessage(const QString& msg);
    QJsonObject params;

protected:
    // void paintEvent(QPaintEvent *event) override;
protected slots:
    void retranslate_ui() final; //更新显示语言
    void showEvent(QShowEvent *) override;;//打开窗口时执行
private:
    void retranslate();// 对自定义组件（如图表） 更新显示,重新翻译
};
extern QPointer<TrolleyWidget>gTrolley;
#endif // TROLLEYWIDGET_H
