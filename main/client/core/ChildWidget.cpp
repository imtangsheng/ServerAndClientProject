#include "ChildWindow/ChildWidget.h"
#include "../MainWindow.h"
ChildWidget::ChildWidget(MainWindow *parent)
    :mainWindow(parent)
{
    connect(mainWindow,&MainWindow::languageChanged,this,&ChildWidget::retranslate_ui);
}

void ChildWidget::initialize()
{
    qDebug() <<_module()<<"#ChildWidget::initialize()";
    if(mainWindow){
        //设备管理界面
        int index = mainWindow->ui.StackedWidgetDeviceItems->addWidget(GetWidgetDeviceManager());
        mainWindow->ui.verticalLayout_DeviceManager_items_name->insertWidget(index,GetButtonDeviceManager());
        connect(GetButtonDeviceManager(),&QRadioButton::clicked,mainWindow,[&]{
            mainWindow->ui.StackedWidgetDeviceItems->setCurrentWidget(GetWidgetDeviceManager());
        });

        currentState.addHandler(DeviceState::Offline,[this](){
            onConnectionChanged(false);//离线状态显示
        });
        currentState.setState(DeviceState::Offline);
        // onConnectionChanged(false);//离线状态显示
        //采集界面
        mainWindow->ui.LayoutRealtimeMonitoring->addWidget(GetWidgetAcquisitionMonitor());
        // 连接的是派生类函数 可用 lambda 表达式显示调用
        connect(mainWindow, &MainWindow::sigTaskConfigChanged,this, &ChildWidget::UpdateTaskConfigSync,Qt::DirectConnection);
    }
}

void ChildWidget::onConnectionChanged(bool enable)
{
    bool isOffline = !enable;
    switch (deviceType) {
    case Trolley:
        if(isOffline){
            mainWindow->ui.widget_device_trolley_is_connected->hide();
            mainWindow->ui.widget_device_trolley_not_connection->show();
        }else{
            mainWindow->ui.widget_device_trolley_is_connected->show();
            mainWindow->ui.widget_device_trolley_not_connection->hide();
        }
        break;
    case Scanner:
        if(isOffline){
            mainWindow->ui.widget_device_scanner_is_connected->hide();
            mainWindow->ui.widget_device_scanner_not_connection->show();
        }else{
            mainWindow->ui.widget_device_scanner_is_connected->show();
            mainWindow->ui.widget_device_scanner_not_connection->hide();
        }
        break;
    case Camera:
        if(isOffline){
            mainWindow->ui.widget_device_camera_is_connected->hide();
            mainWindow->ui.widget_device_network_not_connection->show();
        }else{
            mainWindow->ui.widget_device_camera_is_connected->show();
            mainWindow->ui.widget_device_network_not_connection->hide();
        }
        break;
    default:
        qWarning()<<"setDeviceState is error:"<<deviceType;
        break;
    }
}

void ChildWidget::stop()
{
    mainWindow->ui.pushButton_acquisition_stop->click();
}
