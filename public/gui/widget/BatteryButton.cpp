#include "BatteryButton.h"
#include <QPainter>
#include <QStyle>
BatteryButton::BatteryButton(QWidget *parent)
    : QPushButton{parent}
{
    // ��ʼ����˸��ʱ��
    connect(&m_blinkTimer, &QTimer::timeout,this, [this]() {
        m_blinkOn = !m_blinkOn;
        update();
    });
    m_blinkTimer.setInterval(500); // Ĭ��500ms��˸���
}

void BatteryButton::setLevel(Level level)
{
    m_level = level;
    // ������ʽ����
    style()->unpolish(this);
    style()->polish(this);
    update();
    emit levelChanged();
}

void BatteryButton::setActive(bool active) {
    if(m_level == Unknown) return;//δ��ʼ����������
    setEnabled(!active); //����ʹ����,�Ͳ����Ե���л���
    m_active = active;
    m_blinkOn = false;
    // m_blinkTimer.start();
    update();
}

void BatteryButton::setValue(quint8 value, double voltage) {
    // ���ݵ����ּ�
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
    // ��ѡ�����°�ť�ı���ʾ�����͵�ѹ
    setText(QString("%1%\n%2v").arg(value).arg(voltage,0,'f',1));
    // �����ػ�
    update();
}

#include <QStylePainter>
#include <QStyleOptionButton>
void BatteryButton::paintEvent(QPaintEvent * event) {
    //������,������,�ʹ�����˸��Ч��
    if (m_active && m_level < Low && m_blinkOn) {
        return;
    }
    //��������ʹ�õ�״̬,��δ����״̬���� ��ʾ��ɫ
    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option); // ��ʼ����ʽѡ��
    // �����ť�����ã���ʾ������ɫ������״̬����ۣ�
    if (!isEnabled()) {
        // ǿ����� State_Enabled ״̬���ý��õİ�ť��ʾΪ�������
        option.state |= QStyle::State_Enabled;
    }else{
        // �Ƴ� State_Enabled ״̬�������õİ�ť��ʾΪ�������
        option.state &= ~QStyle::State_Enabled;
    }
    // ���õ�ɫ���е�������ɫ
    p.drawControl(QStyle::CE_PushButton, option); // ���ư�ť
}
