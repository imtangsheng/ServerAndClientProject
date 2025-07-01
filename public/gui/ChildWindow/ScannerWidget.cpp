#include "mainwindow.h"
#include "ScannerWidget.h"
// #include "ui_ScannerWidget.h"

ScannerWidget::ScannerWidget(MainWindow* parent)
    : ChildWidget(parent)
    , ui(new Ui::ScannerWidget)
{
    ui->setupUi(this);

    deviceType = Scanner;
    ChildWidget::initialize();

    gShare.RegisterHandler(_module(),this);
}

ScannerWidget::~ScannerWidget()
{
    qDebug() << "FaroWidget::FaroWidget";
    delete ui;
}

void ScannerWidget::initialize()
{

}

QString ScannerWidget::_module() const
{
    static QString module = share::Shared::GetModuleName(share::ModuleName::scanner);
    return module;
}


void ScannerWidget::ShowMessage(const QString &msg)
{
qDebug() <<"ShowMessage:"<< msg;
}

void ScannerWidget::initUi(const Session &session)
{
    QJsonObject obj = session.result.toObject();
    if(obj.isEmpty()) return;
    isInitUi = true;
}

void ScannerWidget::onConfigChanged(QJsonObject config)
{
    config_ = config;
    parameter = config_.value("params").toObject();
    task = config_.value("task").toObject();
    general = config_.value("general").toObject();
}

void ScannerWidget::onDeviceStateChanged(double state, QString message)
{
    if(!isInitUi){
        gControl.sendTextMessage(Session::RequestString(11,_module(),"initUi",state));
    }
    return ChildWidget::onDeviceStateChanged(state,message);
}

QRadioButton *ScannerWidget::GetButtonDeviceManager()
{
    return ui->ButtonMenu_DeviceManager;
}

QWidget *ScannerWidget::GetWidgetDeviceManager()
{
    return ui->widget_DeviceManager;
}

QWidget *ScannerWidget::GetWidgetAcquisitionMonitor()
{
    return ui->AcquisitionMonitor;
}

void ScannerWidget::retranslate_ui()
{
    ui->retranslateUi(this);
}
