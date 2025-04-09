#include "TrolleyWidget.h"

TrolleyWidget::TrolleyWidget(MainWindow *parent)
    : IWidget(parent)
    , ui(new Ui::TrolleyWidget)
{
    ui->setupUi(this);
    buttonDeviceManager_ = ui->ButtonMenu_DeviceManager;
    widgetDeviceManager_ = ui->widget_DeviceManager;
    widgetAcquisitionMonitor_ = ui->AcquisitionMonitor;
    type = Trolley;
    IWidget::initialize();
    gSouth.RegisterHandler(sModuleTrolley,this);

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

void TrolleyWidget::prepare()
{

}

void TrolleyWidget::start()
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

void TrolleyWidget::stop()
{

}

void TrolleyWidget::ShowMessage(const QString &msg)
{
    qDebug() <<"ShowMessage:"<< msg;
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
