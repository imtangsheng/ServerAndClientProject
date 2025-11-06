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
    // 注册当前类的成员函数作为处理器
    gShare.handlerBinarySession[share::ModuleName::scanner] = [this](const QByteArray& bytes) {
        qDebug() <<_module() << "gShare.handlerBinarySession[] = [this](const QByteArray& bytes"<< bytes.size();
        handle_binary_message(bytes);
    };
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

Result ScannerWidget::SetTaskParameter(QJsonObject &data)
{
    if(!isConnection){
        ToolTip tip(ToolTip::Confirm,tr("扫描仪设备未连接"),tr("扫描仪未连接,确认任务参数失败,请确认连接状态"),-1,this);
        mainWindow->is_create_new_task_error = true;
        return QDialog::Rejected == tip.exec();
    }
    //获取扫描仪时间同步,每次任务前
    Session session_car(sModuleSerial,"ScanAutomationTimeSync",true);
    if (!gControl.SendAndWaitResult(session_car,tr("扫描仪与小车时间同步"),tr("正在扫描仪与小车时间同步"))) {
        ToolTip::ShowText(tr("错误:设置扫描仪与小车时间同步失败,请重新尝试创建任务"), -1);
        mainWindow->is_create_new_task_error = true;
        return false;
    }

    Session session(_module(), "SetParameter", data);
    if (!gControl.SendAndWaitResult(session,tr("等待扫描仪确认参数"),tr("参数确认"))) {
        ToolTip::ShowText(tr("设置参数失败"), -1);
        mainWindow->is_create_new_task_error = true;
        return Result::Failure(tr("设置参数失败"));
    }

    return true;
}

void ScannerWidget::UpdateTaskConfigSync(QJsonObject &content)
{
    qDebug() <<"# ScannerWidget::UpdateTaskConfigSync(QJsonObject &"<<content;
    Session session(_module(), "GetSerialNumber");
    if (gControl.SendAndWaitResult(session)) {
        content[JSON_SCAN_SN] = session.result.toString();
    }else{
        QString sn = ui->lineEdit_serial_number->text();
        ToolTip::ShowText(tr("扫描仪序列号更新失败:%1,使用最近更新值:%2").arg(session.message,sn),5);
        content[JSON_SCAN_SN] = sn;
    }

    // content[OriginalScanDir] = gTaskFileInfo->path + "/" + kTaskDirPointCloudName;
    clearImageItem();//清空之前的任务预览的图片信息
}


void ScannerWidget::ShowMessage(const QString &msg)
{
qDebug() <<"ShowMessage:"<< msg;
}

void ScannerWidget::ScanPowerSwitch(bool open)
{
    if(open){
        ui->pushButton_power_switch->setText(tr("开启上电"));
        ui->pushButton_power_switch->setStyleSheet("");
    }else{
        ui->pushButton_power_switch->setText(tr("关闭上电"));
        ui->pushButton_power_switch->setStyleSheet(R"(
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(250, 77, 100, 255),
                stop:1 rgba(240, 112, 79, 255));
        )");
    }

    isPowerSupply = open;
}

void ScannerWidget::onConnectionChanged(bool enable)
{
    qDebug() <<_module()<< "模块已经加载,指令控制状态"<<enable;
    ChildWidget::onConnectionChanged(enable);
    if(enable){
        ui->pushButton_connect->hide();
    } else {
        ui->pushButton_connect->show();
    }
}

void ScannerWidget::onUpdateUi(const QJsonObject& obj)
{
    qDebug() <<_module()<< "nUpdateUi"<<obj;
    if(obj.isEmpty()) return;
    QString ip = obj.value("ip").toString();
    if(!ip.isEmpty()) ui->lineEdit_scanner_ip->setText(ip);
    //序列号更新
    QString sn = obj.value("serial-number").toString();
    if(!sn.isEmpty()) ui->lineEdit_serial_number->setText(sn);

}

void ScannerWidget::onConfigChanged(QJsonObject config)
{
    config_ = config;
    parameter = config_.value("params").toObject();
    task = config_.value("task").toObject();
    general = config_.value("general").toObject();
}

void ScannerWidget::onDeviceStateChanged(double state)
{
    return ChildWidget::onDeviceStateChanged(state);
}

void ScannerWidget::onWatcherFilesChanged(QJsonObject obj)
{
    qDebug() <<"onWatcherFilesChanged:"<< obj;
    static qint64 latency = QDateTime::currentMSecsSinceEpoch();
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    ui->label_update_delay->setText(QString("%1").arg(currentTime-latency));
    ui->label_update_delay->update();
    latency = currentTime;

    QString filenames = obj.value("FileNames").toString();
    if(filenames.isEmpty()) return;
    ui->plainTextEdit_WatcherFiles->setPlainText(filenames);
    ui->plainTextEdit_WatcherFiles->moveCursor(QTextCursor::End);
}

void ScannerWidget::handle_binary_message(const QByteArray &bytes)
{
    //接收二进制图片文件
    addImageItem(bytes);
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

void ScannerWidget::on_pushButton_update_clicked()
{
    // Session session(_module(), "onUpdateUi");
    // if (!gControl.SendAndWaitResult(session)) {
    //     ToolTip::ShowText(tr("更新信息失败"), -1);
    //     return;
    // }
    Session session(_module(), "execute",QJsonArray{"test"});
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(tr("execute更新信息失败"), -1);
        return;
    }

}

void ScannerWidget::on_pushButton_serial_number_update_clicked()
{
    Session session(_module(), "GetSerialNumber");
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(tr("更新信息失败"), -1);
        return;
    }
    QString sn = session.result.toString();
    ui->lineEdit_serial_number->setText(sn);
}

void ScannerWidget::on_comboBox_param_templates_activated(int index)
{
    parameter = mainWindow->parameterTemplatesInfo.at(index).toObject();
}


void ScannerWidget::on_pushButton_power_switch_clicked()
{
    qDebug() <<"void ScannerWidget::on_pushButton_power_on_clicked()"<<isPowerSupply;
    thread_local int isSupply;
    if(isPowerSupply) {
        if(ToolTip::ShowText(tr("请确认是否关闭扫描仪供电"), -1) != QDialog::Accepted)
        return;
    }
    QJsonObject obj;
    obj["code"] = 0x11;//CAR_SCANER_PWR_CTRL
    QByteArray data = QByteArray(1, static_cast<char>(isPowerSupply ? 0x00 : 0x01));//01 为打开电源，00 为关闭电源。
    obj["data"] = QString(data.toHex());
    Session session(sModuleSerial, "SetParameterByCode", obj);
    if (!gControl.SendAndWaitResult(session,tr("等待扫描仪设置供电"))) {
        ToolTip::ShowText(tr("扫描仪供电设置失败"), -1);
        return;
    }
    ScanPowerSwitch(!isPowerSupply);
}


void ScannerWidget::on_pushButton_power_off_clicked()
{
    Session session(_module(), "Shutdown");
    gControl.sendTextMessage(session.GetRequest());

}

#include "../dialog/CameraFocalLengthDialog.h"
void ScannerWidget::on_pushButton_diameter_height_measurement_clicked()
{
    CameraFocalLengthDialog dialog(this);
    dialog.exec();
}


void ScannerWidget::on_pushButton_start_clicked()
{
    /*状态机方法结构最清晰，适合复杂的状态转换逻辑 但是需要继承类自定义*/
    QString cmd,text,style;
    static int currentState = 1;
    int state;
    QString tip;
    switch (currentState) {
    case 1:
        cmd = "ScanStart";tip = tr("启动失败");
        state = 2;text = tr("开始\n录制");
        style = "font-size: 30px;";
        break;
    case 2:
        cmd = "ScanRecord";tip = tr("开始录制数据失败");
        state = 4;text = tr("停止\n扫描");
        style = R"(
            font-size: 30px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                  stop:0 rgba(250, 77, 100, 255),
                  stop:1 rgba(240, 112, 79, 255));
        )";
        break;
    case 4:
        cmd = "ScanStop";tip = tr("立即停止扫描失败");
        state = 1;text = tr("开始\n扫描");
        style = "font-size: 30px;";
        break;
    }
    Session session(_module(), cmd);
    if (gControl.SendAndWaitResult(session)) {
        currentState = state;
        ui->pushButton_start->setText(text);
        ui->pushButton_start->setStyleSheet(style);
    } else {
        ToolTip::ShowText(tip, -1);
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

void ScannerWidget::on_radioButton_rate_1_clicked()
{
    parameter[Json_Quality] = 4;
    parameter[Json_MeasurementRate] = 1;
    ui->radioButton_quality_4->setChecked(true);


}


void ScannerWidget::on_radioButton_rate_2_clicked()
{
    parameter[Json_Quality] = 3;
    parameter[Json_MeasurementRate] = 2;
    ui->radioButton_quality_3->setChecked(true);

}


void ScannerWidget::on_radioButton_rate_4_clicked()
{
    parameter[Json_Quality] = 2;
    parameter[Json_MeasurementRate] = 4;
    ui->radioButton_quality_2->setChecked(true);

}


void ScannerWidget::on_radioButton_rate_8_clicked()
{
    parameter[Json_Quality] = 1;
    parameter[Json_MeasurementRate] = 8;
    ui->radioButton_quality_1->setChecked(true);

}

void ScannerWidget::on_radioButton_quality_1_clicked()
{
    parameter[Json_Quality] = 1;
    parameter[Json_MeasurementRate] = 8;
    ui->radioButton_rate_8->setChecked(true);
}


void ScannerWidget::on_radioButton_quality_2_clicked()
{
    parameter[Json_Quality] = 2;
    parameter[Json_MeasurementRate] = 4;
    ui->radioButton_rate_4->setChecked(true);

}


void ScannerWidget::on_radioButton_quality_3_clicked()
{
    parameter[Json_Quality] = 3;
    parameter[Json_MeasurementRate] = 2;
    ui->radioButton_rate_2->setChecked(true);
}


void ScannerWidget::on_radioButton_quality_4_clicked()
{
    parameter[Json_Quality] = 4;
    parameter[Json_MeasurementRate] = 1;
    ui->radioButton_rate_1->setChecked(true);
}


void ScannerWidget::on_radioButton_quality_6_clicked()
{
parameter[Json_Quality] = 6;
}


void ScannerWidget::on_radioButton_quality_8_clicked()
{
parameter[Json_Quality] = 8;
}


void ScannerWidget::on_radioButton_resolution_1_clicked()
{
    parameter[Json_Resolution] = 1;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(true);
    ui->radioButton_rate_4->setEnabled(true);
    ui->radioButton_rate_8->setEnabled(true);

    ui->radioButton_quality_1->setEnabled(true);
    ui->radioButton_quality_2->setEnabled(true);
    ui->radioButton_quality_3->setEnabled(true);
    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);
}


void ScannerWidget::on_radioButton_resolution_2_clicked()
{
    parameter[Json_Resolution] = 2;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(true);
    ui->radioButton_rate_4->setEnabled(true);
    ui->radioButton_rate_8->setEnabled(true);

    ui->radioButton_quality_1->setEnabled(true);
    ui->radioButton_quality_2->setEnabled(true);
    ui->radioButton_quality_3->setEnabled(true);
    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);
}


void ScannerWidget::on_radioButton_resolution_4_clicked()
{
    parameter[Json_Resolution] = 4;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(true);
    ui->radioButton_rate_4->setEnabled(true);
    ui->radioButton_rate_8->setEnabled(true);

    ui->radioButton_quality_1->setEnabled(true);
    ui->radioButton_quality_2->setEnabled(true);
    ui->radioButton_quality_3->setEnabled(true);
    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);
}


void ScannerWidget::on_radioButton_resolution_5_clicked()
{
    parameter[Json_Resolution] = 5;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(true);
    ui->radioButton_rate_4->setEnabled(true);
    ui->radioButton_rate_8->setEnabled(false);

    ui->radioButton_quality_1->setEnabled(false);
    ui->radioButton_quality_2->setEnabled(true);
    ui->radioButton_quality_3->setEnabled(true);
    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);
}


void ScannerWidget::on_radioButton_resolution_8_clicked()
{
    parameter[Json_Resolution] = 8;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(true);
    ui->radioButton_rate_4->setEnabled(true);
    ui->radioButton_rate_8->setEnabled(false);

    ui->radioButton_quality_1->setEnabled(false);
    ui->radioButton_quality_2->setEnabled(true);
    ui->radioButton_quality_3->setEnabled(true);
    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);
}


void ScannerWidget::on_radioButton_resolution_10_clicked()
{
    parameter[Json_Resolution] = 10;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(true);
    ui->radioButton_rate_4->setEnabled(false);
    ui->radioButton_rate_8->setEnabled(false);

    ui->radioButton_quality_1->setEnabled(false);
    ui->radioButton_quality_2->setEnabled(false);
    ui->radioButton_quality_3->setEnabled(true);
    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);
}


void ScannerWidget::on_radioButton_resolution_16_clicked()
{
    parameter[Json_Resolution] = 16;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(true);
    ui->radioButton_rate_4->setEnabled(false);
    ui->radioButton_rate_8->setEnabled(false);

    ui->radioButton_quality_1->setEnabled(false);
    ui->radioButton_quality_2->setEnabled(false);
    ui->radioButton_quality_3->setEnabled(true);
    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);
}


void ScannerWidget::on_radioButton_resolution_20_clicked()
{
    parameter[Json_Resolution] = 20;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(false);
    ui->radioButton_rate_4->setEnabled(false);
    ui->radioButton_rate_8->setEnabled(false);

    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_1->setEnabled(false);
    ui->radioButton_quality_2->setEnabled(false);
    ui->radioButton_quality_3->setEnabled(false);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);
}


void ScannerWidget::on_radioButton_resolution_32_clicked()
{
    parameter[Json_Resolution] = 32;
    ui->radioButton_rate_1->setEnabled(true);
    ui->radioButton_rate_2->setEnabled(false);
    ui->radioButton_rate_4->setEnabled(false);
    ui->radioButton_rate_8->setEnabled(false);

    ui->radioButton_quality_4->setEnabled(true);
    ui->radioButton_quality_1->setEnabled(false);
    ui->radioButton_quality_2->setEnabled(false);
    ui->radioButton_quality_3->setEnabled(false);
    ui->radioButton_quality_6->setEnabled(false);
    ui->radioButton_quality_8->setEnabled(false);

}


void ScannerWidget::on_pushButton_ScanStart_clicked()
{
    QString cmd,text;
    cmd = "ScanStart";text = tr("启动失败:");
    Session session(_module(), cmd);
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(text+session.message, -1);
    }
}


void ScannerWidget::on_pushButton_ScanRecord_clicked()
{
    Session session(_module(), "ScanRecord");
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(session.message, -1);
    }
}


void ScannerWidget::on_pushButton_ScanPause_clicked()
{
    Session session(_module(), "ScanPause");
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(session.message, -1);
    }
}


void ScannerWidget::on_pushButton_ScanStop_clicked()
{
    Session session(_module(), "ScanStop");
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(session.message, -1);
    }
}


void ScannerWidget::on_pushButton_ScanSetParameter_clicked()
{
    parameter[JSON_OriginalScanDir] = gShare.info.value(JSON_OriginalScanDir).toString() + "../faro/data";
    // SetTaskParameter(parameter);
    Session session(_module(), "SetParameter", parameter);
    if (!gControl.SendAndWaitResult(session,tr("设置扫描仪参数"))) {
        ToolTip::ShowText(session.message, -1);
    }
}


void ScannerWidget::on_radioButton_monitor_files_clicked()
{
    ui->stackedWidget_MonitorInfo->setCurrentWidget(ui->page_WatcherFiles);
}


void ScannerWidget::on_radioButton_monitor_image_clicked()
{
    ui->stackedWidget_MonitorInfo->setCurrentWidget(ui->page_WatcherImage);
}

void ScannerWidget::addImageItem(const QByteArray &data)
{
    if (data.isEmpty()) {
        qWarning() << "数据为空";
        return;
    }
    QDataStream stream(data);
    QString name;
    QByteArray fileBytes;
    stream >> name >> fileBytes;
    // 从字节数组创建 QPixmap
    QPixmap pixmap;
    if (!pixmap.loadFromData(fileBytes)) {
        qWarning() << "Failed to load image from data";
        return;
    }
    // 创建缩略图（可选，用于优化显示性能）
    // QPixmap thumbnail = pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // 创建 QListWidgetItem
    QListWidgetItem *item = new QListWidgetItem();
    item->setIcon(QIcon(pixmap));
    item->setText(name); // 可选：设置显示文本
    // 设置文字居中对齐
    item->setTextAlignment(Qt::AlignCenter);
    ui->listWidgetImage->addItem(item);
    ui->listWidgetImage->scrollToItem(item);// 滚动到最新项
}

void ScannerWidget::clearImageItem()
{
    ui->listWidgetImage->clear();
    qDebug() << "已清除所有图片项";
}

