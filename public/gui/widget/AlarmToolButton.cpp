#include "AlarmToolButton.h"
#include <QPainter>
#include <QStyle>

AlarmToolButton::AlarmToolButton(QWidget *parent)
    : QToolButton{parent}
{
    // 初始化闪烁定时器
    connect(&m_blinkTimer, &QTimer::timeout, this, &AlarmToolButton::update_blink_state);
    m_blinkTimer.setInterval(500); // 默认500ms闪烁间隔
}

void AlarmToolButton::setAlarmState(AlarmState state)
{
    if (m_state != state) {
        m_state = state;
        static bool hasError = false;
        // 控制闪烁逻辑
        if (m_state == Normal) {
            this->setText(tr("正常"));
            m_blinkTimer.stop();
            m_blinkOn = false;
            hasError = false;
        } else {
            if(m_state == Error){
                this->setText(tr("错误"));
                hasError = true;
            }
            else if(!hasError && m_state == Warning){
                this->setText(tr("警告"));
            }
            m_blinkTimer.start();
        }

        // 触发样式更新
        style()->unpolish(this);
        style()->polish(this);
        update();

        emit stateChanged(m_state);
    }
}

void AlarmToolButton::mousePressEvent(QMouseEvent *event)
{
    if (m_state != Normal) {
        setAlarmState(Normal); // 点击后重置状态
    }
    QToolButton::mousePressEvent(event);
}

void AlarmToolButton::paintEvent(QPaintEvent *event)
{
    if(m_state == Normal || m_blinkOn) //只在正常情况和异常时候 闪烁
    QToolButton::paintEvent(event);
}

void AlarmToolButton::update_blink_state()
{
    m_blinkOn = !m_blinkOn;
    update();
}
