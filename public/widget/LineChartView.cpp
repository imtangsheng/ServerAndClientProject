#include "LineChartView.h"

static constexpr int AxisXMax = 5;
static constexpr int AxisYmax = 10;
static constexpr int AxisTickCount = 6;

void LineChartView::init()
{
    setChart(new QChart());
    /*X轴初始化*/
    axisX = new QValueAxis(this);
    axisX->setRange(0, AxisXMax);
    axisX->setTickCount(AxisTickCount);// 网格线数
    axisX->setLabelFormat("%0.1f");//设置为显示小数点后1位数
    axisX->setGridLineVisible(false);// 移除网格线
    chart()->addAxis(axisX, Qt::AlignBottom);
    /*Y轴初始化*/
    axisY = new QValueAxis(this);
    axisY->setRange(0, AxisYmax);
    axisY->setTickCount(AxisTickCount);
    axisY->setMinorTickCount(AxisTickCount);
    axisY->setLabelFormat("%0.2f");
    axisY->setGridLineVisible(true);// 主网格线
    axisY->setGridLineColor(Qt::darkGray);
    axisY->setMinorGridLineVisible(true);// 次网格线
    //axisY->setMinorGridLineColor(Qt::gray);
    //axisY->setLabelsAngle(-45);//倾斜45度
    QPen minorPen(Qt::gray);  // 创建一个QPen对象设置颜色为灰色
    QVector<qreal> dashes;// 自定义虚线样式
    dashes << 10 << 5;  // 像素实线，像素空白
    minorPen.setDashPattern(dashes);
    //minorPen.setStyle(Qt::DashLine);  // 设置为虚线
    // 或者使用Qt::DotLine设置为点线
    // 或者使用Qt::DashDotLine设置为点划线
    // 设置次网格线的画笔
    axisY->setMinorGridLinePen(minorPen);
    chart()->addAxis(axisY, Qt::AlignLeft);
    /*折线图初始化*/
    lineSeriesUpper = new QLineSeries(this);
    lineSeriesLower = new QLineSeries(this);

    areaSeries = new QAreaSeries(lineSeriesUpper, lineSeriesLower);
    // 创建线性渐变效果
    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    // 添加多个渐变停止点
    gradient.setColorAt(0.0, QColor(0, 100, 255, 255));  // 顶部
    gradient.setColorAt(0.3, QColor(0, 100, 255, 200));  // 上部
    gradient.setColorAt(0.6, QColor(0, 100, 255, 150));  // 中部
    gradient.setColorAt(0.8, QColor(0, 100, 255, 100));  // 下部
    gradient.setColorAt(1.0, QColor(0, 100, 255, 50));   // 底部

    areaSeries->setBrush(gradient);
    // 设置边界线颜色为纯蓝色
    // areaSeries->setColor(QColor(0, 100, 255, 100));  // 半透明蓝色
    areaSeries->setBorderColor(Qt::blue);            // 边界线颜色

    // 将区域图添加到图表
    chart()->addSeries(areaSeries); // 先添加区域，确保在下方
    // 关键：将areaSeries附加到坐标轴
    areaSeries->attachAxis(axisX);
    areaSeries->attachAxis(axisY);
    /*表的初始化*/
    chart()->legend()->hide();                 //隐藏图例
    chart()->setContentsMargins(0, 0, 0, 0);  //设置外边界全部为 int left, int top, int right, int bottom
    //留白设置坐标轴标题
    chart()->setMargins(QMargins(0, 9, 42,0));//设置内边界全部为 int left, int top, int right, int bottom
    chart()->setBackgroundRoundness(0);       //设置背景区域无圆角
    // 设置图表背景透明
    chart()->setBackgroundVisible(false);
    chart()->setPlotAreaBackgroundVisible(false);

    /*显示区域的初始化*/
    // setContentsMargins(0, 0, 0, 0);
}

void LineChartView::append(const double &time, const double &mileage)
{
    // if(!chart()) return;
    lineSeriesUpper->append(time,mileage);
    lineSeriesLower->append(time,0);
    axisX->setMax(time);
    axisY->setMax(mileage);
}

void LineChartView::clear()
{
    lineSeriesUpper->clear();
    lineSeriesLower->clear();
    axisX->setMax(AxisXMax);
    axisY->setMax(AxisYmax);
}
