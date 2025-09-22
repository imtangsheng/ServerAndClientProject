#include "BatteryButton.h"
#include <QPainter>
#include <QStyle>
BatteryButton::BatteryButton(QWidget *parent)
    : QPushButton{parent}
{
    // 初始化闪烁定时器
    connect(&m_blinkTimer, &QTimer::timeout,this, [this]() {
        m_blinkOn = !m_blinkOn;
        update();
    });
    m_blinkTimer.setInterval(500); // 默认500ms闪烁间隔
}

void BatteryButton::setLevel(Level level)
{
    m_level = level;
    // 触发样式更新
    style()->unpolish(this);
    style()->polish(this);
    update();
    emit levelChanged();
}

void BatteryButton::setActive(bool active) {
    if(m_level == Unknown) return;//未初始化不可设置
    setEnabled(!active); //激活使用了,就不可以点击切换了
    m_active = active;
    m_blinkOn = false;
    // m_blinkTimer.start();
    update();
}

void BatteryButton::setValue(quint8 value, double voltage) {
    // 根据电量分级
    Level newLevel;
    if (value >= 80) {
        newLevel = Full;
    } else if (value >= 60) {
        newLevel = High;
    } else if (value >= 30) {
        newLevel = Medium;
    } else if (value >= 10) {
        newLevel = Low;
    } else {
        newLevel = VeryLow;
    }
    if(newLevel != m_level){
        if(value <= 10) m_blinkTimer.start();
        else if(value >=50 )m_blinkTimer.stop();
        setLevel(newLevel);
    }
    // 可选：更新按钮文本显示电量和电压
    setText(QString("%1%\n%2v").arg(value).arg(voltage,0,'f',1));
    // 触发重绘
    update();
}

#include <QStylePainter>
#include <QStyleOptionButton>
void BatteryButton::paintEvent(QPaintEvent * event) {
    //激活了,电量低,就触发闪烁的效果
    if (m_active && m_level < Low && m_blinkOn) {
        return;
    }
    //不是正在使用的状态,按未禁用状态处理 显示暗色
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option); // 初始化样式选项
    // 如果按钮被禁用，显示正常颜色（启用状态的外观）
    if (!isEnabled()) {
        // 强制添加 State_Enabled 状态，让禁用的按钮显示为启用外观
        option.state |= QStyle::State_Enabled;
    }else{
        // 移除 State_Enabled 状态，让启用的按钮显示为禁用外观
        option.state &= ~QStyle::State_Enabled;
    }
    // 设置调色板中的文字颜色
    p.drawControl(QStyle::CE_PushButton, option); // 绘制按钮
}
