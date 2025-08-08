#include "mainwindow.h"
#include "ScannerWidget.h"
// #include "ui_ScannerWidget.h"

ScannerWidget::ScannerWidget(MainWindow* parent)
    : ChildWidget(parent)
    , ui(new Ui::ScannerWidget)
{
    ui->setupUi(this);

    ui->comboBox_param_templates->setModel(&parent->paramNamesModel);
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

void ScannerWidget::onWatcherFilesChanged(QJsonObject obj)
{
    qDebug() <<"file changed:"<< obj;
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

void ScannerWidget::on_comboBox_param_templates_activated(int index)
{
    parameter = mainWindow->parameterTemplatesInfo.at(index).toObject();
}


void ScannerWidget::on_pushButton_power_on_clicked()
{
    qDebug() <<"void ScannerWidget::on_pushButton_power_on_clicked()";
    Session session(_module(), "GetProgressPercent");
    if (gControl.SendAndWaitResult(session)) {
        int percent = session.params.toInt();
        qDebug() <<"扫描进度(%):" << percent;
    } else {
        ToolTip::ShowText(tr("获取进度失败"), -1);
    }
}


void ScannerWidget::on_pushButton_power_off_clicked()
{
    QJsonArray param;
    param.append("shutdown");
    Session session(_module(), "execute",param);
    gControl.sendTextMessage(session.GetRequest());

}


void ScannerWidget::on_pushButton_diameter_height_measurement_clicked()
{
    QJsonObject param;
    param["dir"] = "D:/Test/Data";
    param["CameraHight"] = 1.8;//相机中心到轨面的高度
    param["partes"] = 15;//15机位 15个分组

    Session session(_module(), "GetCameraPositionDistance",param);
    gControl.sendTextMessage(session.GetRequest());
    // if (gControl.SendAndWaitResult(session)) {
    // } else {
        // ToolTip::ShowText(tr("测量相机焦距失败"), -1);
    // }
}


void ScannerWidget::on_pushButton_task_management_clicked()
{
    Session session(_module(), "ScanPause");
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(tr("暂停扫描失败"), -1);
    }
}


void ScannerWidget::on_pushButton_start_clicked()
{
    QString cmd,text;
    static int state =1;
    switch (state) {
    case 1:
        cmd = "ScanStart";text = tr("启动失败");state = 2;
        break;
    case 2:
        cmd = "ScanRecord";text = tr("开始录制数据失败");state = 4;
        break;
    case 4:
        cmd = "ScanStop";text = tr("立即停止扫描失败"); state = 1;
        break;
    }
    Session session(_module(), cmd);
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(text, -1);
    }
}


void ScannerWidget::on_pushButton_connect_clicked()
{
    QJsonObject obj;
    obj["ip"] = ui->lineEdit_scanner_ip->text();
    obj["name"] = _module();
    Session session(sModuleManager, "Activate",obj); //设备激活使用
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(tr("设备连接激活失败"), -1);
    }
}


void ScannerWidget::on_radioButton_rate_1_clicked()
{
    Session session(_module(), "SetParameter", parameter);
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(tr("设置参数失败"), -1);
    }
}


void ScannerWidget::on_radioButton_rate_2_clicked()
{
    QString cmd,text;
    cmd = "ScanStart";text = tr("启动失败");
    Session session(_module(), cmd);
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(text, -1);
    }
}


void ScannerWidget::on_radioButton_rate_4_clicked()
{

    QString cmd,text;
    cmd = "ScanRecord";text = tr("开始录制数据失败");

    Session session(_module(), cmd);
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(text, -1);
    }
}


void ScannerWidget::on_radioButton_rate_8_clicked()
{
    Session session(_module(), "ScanStop");
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(tr("立即停止扫描失败"), -1);
    }
}


void ScannerWidget::on_pushButton_update_clicked()
{
    Session session(_module(), "initUi");
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(tr("更新信息失败"), -1);
        return;
    }
}


void ScannerWidget::on_spinBox_number_scan_lines_editingFinished()
{
    parameter[Json_NumCols] = ui->spinBox_number_scan_lines->value();
}


void ScannerWidget::on_spinBox_number_block_lines_editingFinished()
{
    parameter[Json_SplitAfterLines] = ui->spinBox_number_block_lines->value();
}

