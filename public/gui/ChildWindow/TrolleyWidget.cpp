#include "TrolleyWidget.h"
#include "MainWindow.h"

#include "ScannerWidget.h"
#ifndef DEVICE_TYPE_CAR
#define DEVICE_TYPE_CAR
#endif
#include "serialport_protocol.h"
using namespace serial;

static CarInfo gCarInfo;
static BatteryInfo gBatteryInfoLeft;
static BatteryInfo gBatteryInfoRight;

inline Session GetSession(const QString& module, const QString& method, FunctionCodeEnum code,QByteArray data)
{
    QJsonObject obj;
    obj["code"] = code;
    obj["data"] = QString(data.toHex());
    return Session(module,method,obj);
}

inline void SessionParameterByCode(FunctionCodeEnum code,QByteArray data,Session &session){
    QJsonObject obj;
    obj["code"] = serial::CAR_SET_SPEED;
    obj["data"] = QString(data.toHex());
    session.params = obj;
}

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

    connect(mainWindow->ui.radioButton_car_forward, &QRadioButton::clicked,this, [this]() {
        if (!SetDirection(true)) {
            mainWindow->ui.radioButton_car_forward->setChecked(false);
        } else {
            ui->radioButton_Direction_Drive->setChecked(true);
        }
    });
    connect(mainWindow->ui.radioButton_car_backward, &QRadioButton::clicked,this, [this]() {
        if(!SetDirection(false)){
            mainWindow->ui.radioButton_car_backward->setChecked(false);
        } else {
            ui->radioButton_Direction_Reverse->setChecked(true);
        }
    });

    connect(mainWindow->ui.pushButton_battery_left,&QPushButton::clicked,this,[this](){
        SetBatterySource(1);
    });
    connect(mainWindow->ui.pushButton_battery_right,&QPushButton::clicked,this,[this](){
        SetBatterySource(2);
    });
}

QString TrolleyWidget::_module() const
{
    static QString module = share::Shared::GetModuleName(share::ModuleName::serial);
    return module;
}

void TrolleyWidget::UpdateCarInfo()
{
    static CarInfo lastCarInfo;
    qDebug() <<"UpdateCarInfo" << gCarInfo.temperature;
    // 温度
    if(lastCarInfo.temperature != gCarInfo.temperature){
        double temp = (gCarInfo.temperature_symbol ? -1 : 1) * gCarInfo.temperature / 10.0;
        mainWindow->ui.label_car_remperature->setText(QString("%1°c").arg(temp,0, 'f', 1));
    }
    //方向检测
    if(lastCarInfo.direction != gCarInfo.direction){
        if(gCarInfo.direction == 0){//运行方向；0 后退，1 前进。
            ui->radioButton_Direction_Reverse->setChecked(true);
            mainWindow->ui.radioButton_car_backward->setChecked(true);
        }else{
            ui->radioButton_Direction_Drive->setChecked(true);
            mainWindow->ui.radioButton_car_forward->setChecked(true);
        }
    }

    //电池使用状态,0 未初始化电池，1 使用左侧电池， 2 使用右侧电池
    if(lastCarInfo.battery_usage != gCarInfo.battery_usage){
        if(gCarInfo.battery_usage == 1){
            mainWindow->ui.pushButton_battery_left->setActive(true);
            mainWindow->ui.pushButton_battery_left_2->setActive(true);
            mainWindow->ui.pushButton_battery_right->setActive(false);
            mainWindow->ui.pushButton_battery_right_2->setActive(false);
        }else if(gCarInfo.battery_usage == 2){
            mainWindow->ui.pushButton_battery_left->setActive(false);
            mainWindow->ui.pushButton_battery_left_2->setActive(false);
            mainWindow->ui.pushButton_battery_right->setActive(true);
            mainWindow->ui.pushButton_battery_right_2->setActive(true);
        }else{
            qWarning() << "gCarInfo.battery_usage = " << gCarInfo.battery_usage;
            mainWindow->ui.pushButton_battery_left->setActive(false);
            mainWindow->ui.pushButton_battery_left_2->setActive(false);
            mainWindow->ui.pushButton_battery_right->setActive(false);
            mainWindow->ui.pushButton_battery_right_2->setActive(false);
        }
    }
    //旋钮状态；1 到 5 分别表示旋钮的状态 1 到 5。

    //为目前使用的是哪个编码器；1 左侧，2 右侧，3 同时使用两个编码器 备注:目前固定值3
    if(gCarInfo.encoder_mode != 3){
        qWarning() << "gCarInfo.encoder_mode:"<<gCarInfo.encoder_mode;
    }
    //扫描仪供电状态；0 不供电（默认），1 供电。（由 0x11 指令控制）
    if(lastCarInfo.scanner_power != gCarInfo.scanner_power){
        if(gScanner)
            gScanner->ScanPowerSwitch(gCarInfo.scanner_power==0);
    }
}


Result TrolleyWidget::AutomationTimeSync()
{
    //进行自动化时间同步,手动同步,如果此命令发送时,有设备未同步,则自动化时间会出错,故把同步流程放到此进入任务前
    static bool is_devices_time_sync = false;
    if(!is_devices_time_sync){
        Session session(sModuleSerial, "SetParameterByCode");
        SessionParameterByCode(serial::AUTOMATION_TIME_SYNC,QByteArray(),session);
        if (gControl.SendAndWaitResult(session,tr("设备自动化时间同步"),tr("正在进行自动化时间同步"))) {
            is_devices_time_sync = true;
        }
    }
    return is_devices_time_sync;
}

Result TrolleyWidget::SetTaskParameter(QJsonObject &data)
{
    qint32 speed = data.value(JSON_SPEED).toInt();

    if(speed < 50 || speed > 5500){
        ToolTip::ShowText(tr("设置的行驶速度超过范围:%1-%2,当前设置%3").arg(50,5500,speed), -1);
        return false;
    }
    Session session(_module(), "SetParameterByCode");
    if(gCarInfo.speed != speed){
        SessionParameterByCode(serial::CAR_SET_SPEED,Serialize::Byte(speed),session);
        if (!gControl.SendAndWaitResult(session,tr("设置行驶速度"),tr("正在设置行驶速度"))) {
            ToolTip::ShowText(tr("设置行驶速度失败"), -1);
            return false;
        }
        // if(!SetSpeed(speed)) return Result::Failure(tr("设置行驶速度失败"));
        gCarInfo.speed = speed;
    }

    return true;
}

void TrolleyWidget::UpdateTaskConfigSync(QJsonObject &content)
{
    qDebug() << "#TrolleyWidget::UpdateTaskConfigSync(QJsonObject &"<<content;
    //小车是否使用额定里程,满足该里程自动停止任务
    isUseRatedMileage = mainWindow->ui.radioButton_task_car_rated_mileage_on->isChecked();
    if (isUseRatedMileage) {
        carRatedMileage  = mainWindow->ui.spinBox_task_car_rated_mileage->value();
        content[JSON_CAR_RATED_MILEAGE] = carRatedMileage ;
    }

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

void TrolleyWidget::AddMileage(quint8 symbol,double mileage, qint64 time_us) const
{
    double time_s = time_us / 1000000.0; //us->s
    qDebug() <<"AddMileage: 时间s-里程m"<< time_s<< mileage;//10us m bug切换方向中,里程是返回的绝对值,所以会减小
    qint8 flag = symbol ? -1:1;
    double car_speed = 0.00;
    double car_mileage = 0;
    qint64 car_time = 0;
    static qint64 car_time_start;//小车行走的时间 us
    static double car_mileage_start; //小车行走的距离
    static double lastTime{0};
    static double lastMileage{0};
    //显示更新时间 10秒没有更新则判断需要重新计算
    static qint64 latency = QDateTime::currentMSecsSinceEpoch();
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qint64 delay = currentTime - latency;
    latency = currentTime;

    if(lastTime <= 0 || delay > 10000){
        car_time_start = time_s;
        car_mileage_start = mileage;
    } else {
        car_speed = flag * ((mileage - lastMileage) / (time_s - lastTime)) * 3600;//单位m/h
        car_mileage = mileage - car_mileage_start;
        car_time = time_s - car_time_start;
    }

    if(gTaskState == TaskState::TaskState_Running){
        ui->label_update_delay->setText(QString("%1").arg(delay));//ms
        ui->label_update_delay->update();
        ui->ChartView_mileage_monitor->append(time_s,mileage);
        ui->label_car_task_speed->setText(QString::number(car_speed,'f',2));
        ui->label_car_task_time->setText(QString::number(car_time));
        ui->label_car_task_mileage->setText(QString::number(car_mileage, 'f', 2));

    }else{
        ui->ChartView_mileage_info->append(time_s,mileage);
        ui->doubleSpinBox_car_speed->setValue(car_speed);
        ui->doubleSpinBox_car_time->setValue(car_time);
        ui->doubleSpinBox_car_mileage->setValue(car_mileage);
    }

    lastTime = time_s;
    lastMileage = mileage;
}

void TrolleyWidget::handle_binary_message(const QByteArray &bytes)
{
    QDataStream stream(bytes);
    stream.setByteOrder(QDataStream::LittleEndian);
    quint8 code;stream >> code;
    switch (code) {
    case CAR_GET_MSG:{
        stream >> gCarInfo;
        UpdateCarInfo();
    }break;
    case CAR_GET_BATTERY_MSG:{
        stream >> gBatteryInfoLeft >> gBatteryInfoRight;
        Result ret = gBatteryInfoLeft.isValid();
        if(!ret){
            ToolTip::ShowText(tr("电池显示异常:%1").arg(ret.message), -1);
            break;
        }
        if(gBatteryInfoLeft.fullChargeCapacity != 0){
            mainWindow->ui.pushButton_battery_left->setValue(gBatteryInfoLeft.GetPercentage(),gBatteryInfoLeft.GetVoltage());
            mainWindow->ui.pushButton_battery_left_2->setValue(gBatteryInfoLeft.GetPercentage(),gBatteryInfoLeft.GetVoltage());
        }
        Result ret2 = gBatteryInfoRight.isValid();
        if(!ret2){
            ToolTip::ShowText(tr("电池显示异常:%1").arg(ret2.message), -1);
            break;
        }
        if(gBatteryInfoRight.fullChargeCapacity != 0){
            mainWindow->ui.pushButton_battery_right->setValue(gBatteryInfoRight.GetPercentage(),gBatteryInfoRight.GetVoltage());
            mainWindow->ui.pushButton_battery_right_2->setValue(gBatteryInfoRight.GetPercentage(),gBatteryInfoRight.GetVoltage());
        }
    }break;
    case 0xF8:{
        quint64 id;
        quint8 symbol;
        double mileage; qint64 time;
        stream >> id >> symbol >> mileage >> time;
        AddMileage(symbol,mileage,time);
        }break;
    default:
        qWarning() << _module()<< "二进制数据模块代码未支持:"<<code;
        break;
    }
}


void TrolleyWidget::onUpdateUi(const QJsonObject& value)
{
    if(value.isEmpty()) return;
    if(value.contains("mileage_multiplier")){
        ui->comboBox_speed_multiplier->clear();
        ui->comboBox_speed_multiplier->addItems(value.value("mileage_multiplier").toString().split(","));
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

void TrolleyWidget::onDeviceStateChanged(double state)
{
    return ChildWidget::onDeviceStateChanged(state);
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

void TrolleyWidget::retranslate_ui()
{
    retranslate();
    ui->retranslateUi(this);
}

void TrolleyWidget::showEvent(QShowEvent *)
{

}


void TrolleyWidget::on_pushButton_save_clicked()
{
    config_["version"] = "0.0.1";
    config_["lastUpdate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    // config_["task"] = task;
    // config_["params"] = parameter;
    config_["general"] = general;
    Session session(_module(),"SaveConfig",config_);
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
        ToolTip::ShowText(tr("设备打开串口失败,错误:%1").arg(session.message), -1);
    }
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
        return;
    }
    ui->ChartView_mileage_info->init();
}


void TrolleyWidget::on_pushButton_stop_clicked()
{
    //00：停止小车 01：停止并清除里程
    QJsonObject obj;
    obj["code"] = serial::CAR_STOP;;
    QByteArray data = QByteArray(1, static_cast<char>(0x00));//01清除里程
    obj["data"] = QString(data.toHex());
    Session session(_module(), "SetParameterByCode", obj);
    if (!gControl.SendAndWaitResult(session)) {
        ToolTip::ShowText(tr("停止小车失败"), -1);
    }
}


void TrolleyWidget::on_radioButton_Direction_Drive_clicked()
{
    // 改变行驶方向向前
    if(!SetDirection(01))
        ui->radioButton_Direction_Drive->setChecked(false);
    else {
        mainWindow->ui.radioButton_car_forward->setChecked(true);
    }
}


void TrolleyWidget::on_radioButton_Direction_Reverse_clicked()
{
    // if (gControl.carDirection == true) {//如果车辆方向不对,则先改变方向(不会影响车辆方向)
    if(!SetDirection(00))
        ui->radioButton_Direction_Reverse->setChecked(false);
    else {
        mainWindow->ui.radioButton_car_backward->setChecked(true);
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
    SetSpeed(speed);
}

void TrolleyWidget::on_pushButton_set_car_speed_multiplier_clicked()
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
    /*ToDo 待定,未支持发送额定里程,到达目标后停止,或者前进后退多少再停止,如果使用检测位置,这个精度无法保证,手动检测*/
}


void TrolleyWidget::on_radioButton_mileage_set_off_clicked()
{
    qDebug() << "TrolleyWidget::on_radioButton_mileage_set_off_clicked()";
}


void TrolleyWidget::retranslate()
{
    static QString time = tr("时间(s)");
    static QString mileage = tr("里程(m)");
    ui->ChartView_mileage_monitor->xTitle = time;
    ui->ChartView_mileage_monitor->yTitle = mileage;

    ui->ChartView_mileage_info->xTitle = time;
    ui->ChartView_mileage_info->yTitle = mileage;
    // 触发更新
    update();
}

Result TrolleyWidget::SetSpeed(qint16 speed)
{
    if(speed < 50 || speed > 5500){
        ToolTip::ShowText(tr("设置的行驶速度超过范围:%1-%2,当前设置%3").arg(50,5500,speed), -1);
        return false;
    }
    QJsonObject obj;
    obj["code"] = serial::CAR_SET_SPEED;
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);  // 设置字节序
    stream << speed;
    obj["data"] = QString(data.toHex());
    qDebug() <<"obj:"<< obj;
    Session session(_module(), "SetParameterByCode", obj);
    if (!gControl.SendAndWaitResult(session,tr("设置行驶速度"))) {
        ToolTip::ShowText(tr("设置行驶速度失败"), -1);
        return false;
    }
    return true;
}

Result TrolleyWidget::SetDirection(bool isForward)
{
    QJsonObject obj;
    obj["code"] = serial::CAR_CHANGING_OVER;//serial::CAR_CHANGING_OVER;
    QByteArray data;
    data.append(isForward ? 0x01:0x00);
    obj["data"] = QString(data.toHex());
    Session session(_module(), "SetParameterByCode", obj);
    if (!gControl.SendAndWaitResult(session)) {
        // gControl.carDirection = false;
        ToolTip::ShowText(tr("设置车辆方向失败"), -1);
        // ui->radioButton_Direction_Reverse->setChecked(false);
        return false;
    }
    return true;
}

Result TrolleyWidget::SetBatterySource(quint8 num)
{
    Session session = GetSession(_module(), "SetParameterByCode",serial::CAR_CHOOSE_BATTERY_SOURCE,Serialize::Byte(num));
    if (!gControl.SendAndWaitResult(session,tr("设置使用的电池侧"))) {
        if(session) return false;//点了取消,暂时不处理
        QString msg;
        switch (session.code) {
        case 0x01:msg = tr("正在使用中,无法切换");break;
        case 0x02:msg = tr("左侧电压低,无法切换");break;
        case 0x03:msg = tr("右侧电压低,无法切换");break;
        case 0x04:msg = tr("参数错误");break;
        default:msg = tr("未知错误码%1").arg(session.code);break;
        }
        ToolTip::ShowText(tr("设置使用的电池侧,失败:%1").arg(msg), -1);
        return false;
    }
    return true;
}


