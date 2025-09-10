#ifndef TITLEBAR_H
#define TITLEBAR_H
#include <QStyleOption>
#include <QPainter>
#include <QStyle>
#include <QMouseEvent>
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_StyledBackground, true);//需要样式化背景支持 在控件的默认paintEvent中自动调用样式引擎 即使没有重写paintEvent，Qt也会为你处理样式表背景绘制
    }
private:
    // void paintEvent(QPaintEvent *event) override {
    //     QStyleOption opt;//创建样式选项对象，它包含了控件的状态信息（是否启用、是否有焦点、鼠标状态等）
    //     opt.initFrom(this);//将当前控件的状态信息填充到样式选项中，包括：控件的几何信息（位置、大小）;状态标志（启用/禁用、鼠标悬停等）;样式表信息;
    //     QPainter p(this);//创建画家对象，用于在当前控件上绘制
    //     style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);//调用当前样式引擎绘制基本控件 PE_Widget：告诉样式引擎这是一个基本widget 样式引擎会根据样式表规则绘制背景、边框等
    //     return QWidget::paintEvent(event);
    // }
    bool isMousePressed;
    QPoint mouseStartMovePos;
    void mousePressEvent(QMouseEvent *event) override{
        if(event->button() == Qt::LeftButton){
            isMousePressed = true;
            mouseStartMovePos = event->pos();
        }
        return QWidget::mousePressEvent(event);
    }
    void mouseMoveEvent(QMouseEvent *event) override{
        if(event->buttons() == Qt::LeftButton && isMousePressed){
            this->window()->move(this->window()->pos() + event->pos() - mouseStartMovePos);
            emit posChange(event->pos());
        }
        return QWidget::mouseMoveEvent(event);
    }
    void mouseReleaseEvent(QMouseEvent *event) override{
        if(event->button() == Qt::LeftButton){
            isMousePressed = false;
        }
        return QWidget::mouseReleaseEvent(event);
    }
    void mouseDoubleClickEvent(QMouseEvent *event) override{
        if(event->button() == Qt::LeftButton){
            if(this->window()->windowState() == Qt::WindowFullScreen){
                this->window()->showNormal();
            }else if(this->window()->windowState() == Qt::WindowMaximized){
                this->window()->showNormal();
            }else{
                this->window()->showFullScreen();
            }
        }
        return QWidget::mouseDoubleClickEvent(event);
    }

    // void showEvent(QShowEvent *event) override;

signals:
    void posChange(const QPoint& pos);
private:
};

#endif // TITLEBAR_H
