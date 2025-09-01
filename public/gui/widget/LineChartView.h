#ifndef LINECHARTVIEW_H
#define LINECHARTVIEW_H

/*QtCharts 模块依赖于 QtCore 和 QtGui。确保你的代码中显式或隐式包含了这些模块*/
// #include <QtCore>
// #include <QtGui> // ✅ 正确引入 QtCharts 中的类
#include <QChartView>
#include <QChart>
#include <QLineSeries>
#include <QAreaSeries>
#include <QValueAxis>
class LineChartView : public QChartView
{
public:
    using QChartView::QChartView; // 使用基类的构造函数
    QValueAxis* axisX{nullptr};
    QValueAxis* axisY{nullptr};
    QLineSeries* lineSeriesUpper{nullptr};
    QLineSeries* lineSeriesLower{nullptr};
    QAreaSeries* areaSeries{nullptr};

    QString xTitle;
    QString yTitle;
    void init();
    void append(const double& time,const double& mileage);
    void clear();

protected:
    void paintEvent(QPaintEvent* event) override {
        // 1. 先调用基类绘制图表内容（包括线条、坐标轴等）
        QChartView::paintEvent(event);
        // 2. 检查是否有有效图表
        if (!chart()) return;
        // 3. 创建绘图工具
        QPainter painter(viewport());
        painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
        // 4. 获取绘图区域信息
        QRectF plotArea = chart()->plotArea();  // 图表数据绘制区域
        // QRectF chartRect = rect();               // 整个视图区域

        // 获取系统默认字体并适当调整
        QFont font = this->font(); // 获取视图的字体
        // font.setPointSize(font.pointSize() * 0.9); // 比默认稍小
        painter.setFont(font);
        // 获取字体度量信息
        QFontMetricsF fm(font);

        // qDebug() <<"LineChartView:图表数据绘制区域 整个视图区域"<< plotArea << chartRect;
        // 5. 绘制X轴标题（水平显示）
        if (!xTitle.isEmpty()) {
            // 计算文本实际需要的空间
            //qreal textWidth = fm.horizontalAdvance(xTitle)/2;
            qreal textHeight = fm.height()/2;
            // 计算绘制位置（底部右下） 文本会向右上方延伸（因为基线在左下）和widget计算不同
            QPointF titlePos(
                plotArea.right() + 9,
                plotArea.bottom() + textHeight
                );
            painter.drawText(titlePos, xTitle);
        }

        // 6. 绘制Y轴标题（水平显示）
        if (!yTitle.isEmpty()) {
            // 计算文本实际需要的空间
            qreal textWidth = fm.horizontalAdvance(yTitle)/2;
            qreal textHeight = fm.height()/2;
            //顶部 左上
            QPointF titlePos(
                plotArea.left() - textWidth,
                plotArea.top() - textHeight - 9
                );
            painter.drawText(titlePos, yTitle);
        }
    }
};

#endif // LINECHARTVIEW_H
