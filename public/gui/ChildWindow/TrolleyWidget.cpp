#include "TrolleyWidget.h"
#include "MainWindow.h"

#ifndef DEVICE_TYPE_CAR
#define DEVICE_TYPE_CAR
#endif
#include "serialport_protocol.h"
using namespace serial;

TrolleyWidget::TrolleyWidget(MainWindow *parent)
    : ChildWidget(parent)
    , ui(new Ui::TrolleyWidget)
{
    ui->setupUi(this);

    deviceType = Trolley;
    ChildWidget::initialize();
    // 界面
    ui->comboBox_car_param_templates->setModel(&parent->paramNamesModel);

    //表的标题设置,初始化
    initialize();
    // start();

    gShare.RegisterHandler(_module(),this);
    // 注册当前类的成员函数作为处理器
    gShare.handlerBinarySession[share::ModuleName::trolley] = [this](const QByteArray& bytes) {
        qDebug() <<_module() << "gShare.handlerBinarySession[] = [this](const QByteArray& bytes"<< bytes.size();
        handle_binary_message(bytes);
    };

}

TrolleyWidget::~TrolleyWidget()
{
    qDebug() << "TrolleyWidget::~TrolleyWidget() 析构函数";
    delete ui;
}

void TrolleyWidget::initialize()
{
    retranslate();
    ui->ChartView_mileage_monitor->init();
    ui->ChartView_mileage_info->init();
}

QString TrolleyWidget::_module() const
{
    static QString module = share::Shared::GetModuleName(share::ModuleName::serial);
    return module;
}


void TrolleyWidget::test()
{
    ui->ChartView_mileage_monitor->clear();
    static QTimer timer;

    static double time{0};
    static double mileage{0};

    // 设置初始显示范围
    // static const double visiblePoints = 10; // 可见点数量
    connect(&timer,&QTimer::timeout,this,[&]{
        qDebug() << "add:" << time<<mileage;
        ui->ChartView_mileage_monitor->append(time,mileage);
        time = time + 1;
        mileage = mileage + 5;
    });

    timer.start(1000);


}


void TrolleyWidget::ShowMessage(const QString &msg)
{
    qDebug() <<"ShowMessage:"<< msg;
}

void TrolleyWidget::AddMileage(double time, double mileage,bool backward)
{
    qDebug() <<"AddMileage:"<< time<< mileage;
    qint8 flag = backward ? -1:1;
    double speed = 0;
    if(lastTime > 0){
         speed = ((mileage - lastMileag) / (time - lastTime)) * 3600;//单位m/h
    }

    if(gTaskState == TaskState::TaskState_Running){
        ui->ChartView_mileage_monitor->append(time,mileage);
        // ui->label_car_task_mileage->setText();
    }else{
        ui->ChartView_mileage_info->append(time,mileage);
        ui->doubleSpinBox_car_speed->setValue(speed*flag);
        ui->doubleSpinBox_car_time->setValue(time);
        ui->doubleSpinBox_car_mileage->setValue(mileage*flag);
    }

    lastTime = time;
    lastMileag = mileage;
}

void TrolleyWidget::handle_binary_message(const QByteArray &bytes)
{
    QDataStream stream(bytes);
    quint8 code;stream >> code;
    switch (code) {
    case 0xF8:{
        quint64 id;
        MileageInfo  mileage;
        stream >> id >>mileage;
        //1000000.0 单位us转s
        AddMileage(mileage.time/1000000,mileage.pulse ,mileage.symbol==0);
        }break;
    default:
        qWarning() << _module()<< "二进制数据模块代码未支持:"<<code;
        break;
    }
}


// void TrolleyWidget::onEnableChanged(bool enable)
// {
//     qDebug() <<_module()<< "模块已经加载,指令控制状态"<<enable;
//     ChildWidget::onEnableChanged(enable);
// }

void TrolleyWidget::initUi(const Session &session)
{
    QJsonObject obj = session.result.toObject();
    if(obj.isEmpty()) return;
    isInitUi = true;
    if(obj.contains("mileage_multiplier")){
        ui->comboBox_speed_multiplier->clear();
        ui->comboBox_speed_multiplier->addItems(obj.value("mileage_multiplier").toString().split(","));
    }
}

void TrolleyWidget::onConfigChanged(QJsonObject config)
{
    config_ = config;

    general = config_.value("general").toObject();

    QString mileage_multiplier = general.value("mileage_multiplier").toString();
    if(!mileage_multiplier.isEmpty()){
        ui->comboBox_speed_multiplier->setCurrentText(mileage_multiplier);
    }
}

void TrolleyWidget::onDeviceStateChanged(double state, QString message)
{
    if(!isInitUi){
        gControl.sendTextMessage(Session::RequestString(11,_module(),"initUi",state));
    }
    return ChildWidget::onDeviceStateChanged(state,message);
}

QRadioButton *TrolleyWidget::GetButtonDeviceManager()
{
    return ui->ButtonMenu_DeviceManager;
}

QWidget *TrolleyWidget::GetWidgetDeviceManager()
{
    return ui->widget_DeviceManager;
}

QWidget *TrolleyWidget::GetWidgetAcquisitionMonitor()
{
    return ui->AcquisitionMonitor;
}

// void TrolleyWidget::paintEvent(QPaintEvent *event)
// {
//     qDebug()<<"TrolleyWidget::paintEvent";

// }

void TrolleyWidget::retranslate_ui()
{
    retranslate();
    ui->retranslateUi(this);
}

void TrolleyWidget::showEvent(QShowEvent *)
{

}

void TrolleyWidget::retranslate()
{
    static QString time = tr("Time (s)");
    static QString mileage = tr("Mileage (m)");
    ui->ChartView_mileage_monitor->xTitle = time;
    ui->ChartView_mileage_monitor->yTitle = mileage;

    ui->ChartView_mileage_info->xTitle = time;
    ui->ChartView_mileage_info->yTitle = mileage;
    // 触发更新
    update();
}


void TrolleyWidget::on_pushButton_save_clicked()
{
    config_["version"] = "0.0.1";
    config_["lastUpdate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    // config_["task"] = task;
    // config_["params"] = parameter;
    config_["general"] = general;
    Session session({ {"id", 11}, {"module", _module()}, {"method", "SaveConfig"}, {"params", config_} });
    gControl.SendAndWaitResult(session);
}


void TrolleyWidget::on_pushButton_scan_clicked()
{
    Session session(_module() ,"scan");
    if(!gControl.SendAndWaitResult(session)){
        qDebug() << "QDialog::Rejected"<<QDialog::Rejected;
        return;
    }
    qDebug() << session.result;
    QString names = session.result.toString();
    qDebug() << "Result scan()"<<names;
    // {
        // QSignalBlocker blocker(ui->comboBox_available_ports);// 阻塞信号避免触发变化
    ui->comboBox_available_ports->clear();
    // }
    ui->comboBox_available_ports->addItems(names.split(","));
}

void TrolleyWidget::on_pushButton_open_clicked()
{
    QJsonObject obj;
    obj["port"] = ui->comboBox_available_ports->currentText();
    obj["name"] = _module();
    Session session(sModuleManager, "Activate",obj); //设备激活使用
    if(!gControl.SendAndWaitResult(session)){
        ToolTip::ShowText(tr("设备打开串口失败:%1").arg(session.message), -1);
    }
    // config_["port"] = port;
}

void TrolleyWidget::on_comboBox_car_param_templates_activated(int index)
{
    parameter = mainWindow->parameterTemplatesInfo.at(index).toObject();
    qDebug() <<"参数是:"<< parameter;
}

void TrolleyWidget::on_pushButton_start_clicked()
{
    QJsonObject obj;
    obj["code"] = serial::CAR_STARTUP;;
    QByteArray data = QByteArray(1, static_cast<char>(0x00));//00：启动小车 01：启动并清除里程
    obj["data"] = QString(data.toHex());
    Session session(_module(), "SetParameterByCode", obj);
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(tr("启动小车失败"), -1);
    }
}


void TrolleyWidget::on_pushButton_stop_clicked()
{
    //00：停止小车 01：停止并清除里程
    QJsonObject obj;
    obj["code"] = serial::CAR_STOP;;
    QByteArray data = QByteArray(1, static_cast<char>(0x00));//00：启动小车 01：启动并清除里程
    obj["data"] = QString(data.toHex());
    Session session(_module(), "SetParameterByCode", obj);
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(tr("停止小车失败"), -1);
    }
}

void TrolleyWidget::on_radioButton_Direction_Drive_clicked()
{
    // 改变行驶方向向前
    if (gControl.carDirection == false) {//如果车辆方向不对,则先改变方向(不会影响车辆方向)
        QJsonObject obj;
        obj["code"] = serial::CAR_CHANGING_OVER;//serial::CAR_CHANGING_OVER;
        // QByteArray data;
        // obj["data"] = data.toStdString().c_str();
        Session session(_module(), "SetParameterByCode", obj);
        if (gControl.SendAndWaitResult(session)) {
            gControl.carDirection = true;
        } else {
            ToolTip::ShowText(tr("设置车辆方向失败"), -1);
            ui->radioButton_Direction_Drive->setChecked(false);
        }
    }
}


void TrolleyWidget::on_radioButton_Direction_Reverse_clicked()
{
    if (gControl.carDirection == true) {//如果车辆方向不对,则先改变方向(不会影响车辆方向)
        QJsonObject obj;
        obj["code"] = serial::CAR_CHANGING_OVER;//serial::CAR_CHANGING_OVER;
        QByteArray data;
        obj["data"] = data.toStdString().c_str();
        Session session(_module(), "SetParameterByCode", obj);
        if (gControl.SendAndWaitResult(session)) {
            gControl.carDirection = false;
        } else {
            ToolTip::ShowText(tr("设置车辆方向失败"), -1);
            ui->radioButton_Direction_Reverse->setChecked(false);
        }
    }
}

void TrolleyWidget::on_horizontalScrollBar_car_travel_speed_valueChanged(int value)
{
    // 阻塞 SpinBox 信号避免循环
    ui->spinBox_car_travel_speed->blockSignals(true);
    ui->spinBox_car_travel_speed->setValue(value);
    ui->spinBox_car_travel_speed->blockSignals(false);
}


void TrolleyWidget::on_spinBox_car_travel_speed_valueChanged(int arg1)
{
    // 阻塞 ScrollBar 信号避免循环
    ui->horizontalScrollBar_car_travel_speed->blockSignals(true);
    ui->horizontalScrollBar_car_travel_speed->setValue(arg1);
    ui->horizontalScrollBar_car_travel_speed->blockSignals(false);
}

void TrolleyWidget::on_pushButton_set_car_speed_clicked()
{
    qint16 speed = ui->spinBox_car_travel_speed->value();
    QJsonObject obj;
    obj["code"] = serial::CAR_SET_SPEED;
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);  // 设置字节序
    stream << speed;
    obj["data"] = QString(data.toHex());
    qDebug() <<"obj:"<< obj;
    Session session(_module(), "SetParameterByCode", obj);
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(tr("设置行驶速度失败"), -1);
    }
}


void TrolleyWidget::on_comboBox_speed_multiplier_activated(int index)
{
    QString multiplier = ui->comboBox_speed_multiplier->currentText();
    general["mileage_multiplier"] = multiplier;
    Session session(_module(), "SetSpeedMultiplier",multiplier);
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(tr("设置车辆里程系数失败"), -1);
    }
}


void TrolleyWidget::on_radioButton_mileage_set_on_clicked()
{
    qDebug() << "TrolleyWidget::on_radioButton_mileage_set_on_clicked()";
}


void TrolleyWidget::on_radioButton_mileage_set_off_clicked()
{
    qDebug() << "TrolleyWidget::on_radioButton_mileage_set_off_clicked()";
}


void TrolleyWidget::on_spinBox_car_mileage_set_max_value_editingFinished()
{

}





