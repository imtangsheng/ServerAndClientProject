#include "TrolleyWidget.h"
#include "MainWindow.h"
TrolleyWidget::TrolleyWidget(MainWindow *parent)
    : ChildWidget(parent)
    , ui(new Ui::TrolleyWidget)
{
    ui->setupUi(this);

    deviceType = Trolley;
    ChildWidget::initialize();

    gShare.RegisterHandler(_module(),this);

    // 界面
    ui->comboBox_car_param_templates->setModel(&parent->paramNamesModel);
    //表的标题设置,初始化
    initialize();
    // start();
}

TrolleyWidget::~TrolleyWidget()
{
    qDebug() << "TrolleyWidget::~TrolleyWidget() 析构函数";
    delete ui;
}

void TrolleyWidget::initialize()
{
    retranslate();
    ui->ChartView->init();
}

QString TrolleyWidget::_module() const
{
    static QString module = share::Shared::GetModuleName(share::ModuleName::serial);
    return module;
}


void TrolleyWidget::test()
{
    ui->ChartView->clear();
    static QTimer timer;

    static double time{0};
    static double mileage{0};

    // 设置初始显示范围
    // static const double visiblePoints = 10; // 可见点数量
    connect(&timer,&QTimer::timeout,this,[&]{
        qDebug() << "add:" << time<<mileage;
        ui->ChartView->append(time,mileage);
        time = time + 1;
        mileage = mileage + 5;
    });

    timer.start(1000);


}


void TrolleyWidget::ShowMessage(const QString &msg)
{
    qDebug() <<"ShowMessage:"<< msg;
}

void TrolleyWidget::initUi(const Session &session)
{
    QJsonObject obj = session.result.toObject();
    if(obj.isEmpty()) return;
    isInitUi = true;
    if(obj.contains("speed_multiplier")){
        ui->comboBox_speed_multiplier->clear();
        ui->comboBox_speed_multiplier->addItems(obj.value("speed_multiplier").toString().split(","));
    }
}

void TrolleyWidget::onConfigChanged(QJsonObject config)
{
    config_ = config;

    general = config_.value("general").toObject();

    QString speed_multiplier = general.value("speed_multiplier").toString();
    if(!speed_multiplier.isEmpty()){
        ui->comboBox_speed_multiplier->setCurrentText(speed_multiplier);
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
    ui->ChartView->xTitle = tr("Time (s)");
    ui->ChartView->yTitle = tr("Mileage (m)");
    // 触发更新
    update();
}

void TrolleyWidget::on_pushButton_set_speed_multiplier_clicked()
{
    QString multiplier = ui->comboBox_speed_multiplier->currentText();
    general["speed_multiplier"] = multiplier;
    Session session({ {"id", 11}, {"module", _module()}, {"method", "SetSpeedMultiplier"}, {"params", multiplier} });
    gControl.SendAndWaitResult(session);
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
    Session session({ {"id", 11}, {"module", _module()}, {"method", "scan"}, {"params", true} });
    if(!gControl.SendAndWaitResult(session)){
        qDebug() << "QDialog::Rejected"<<QDialog::Rejected;
        return;
    }
    qDebug() << session.result;
    QString names = session.result.toString();
    qDebug() << "Result scan()"<<names;
    {
        QSignalBlocker blocker(ui->comboBox_device_list);// 阻塞信号避免触发变化
        ui->comboBox_device_list->clear();
    }
    ui->comboBox_device_list->addItems(names.split(","));
}

void TrolleyWidget::on_pushButton_open_clicked()
{
    QString port = ui->comboBox_device_list->currentText();
    Session session({ {"id", 11}, {"module", _module()}, {"method", "open"}, {"params", port} });
    if(gControl.SendAndWaitResult(session)){
        config_["port"] = port;
    }

}
