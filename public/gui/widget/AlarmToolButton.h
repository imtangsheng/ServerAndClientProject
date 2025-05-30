#ifndef ALARMTOOLBUTTON_H
#define ALARMTOOLBUTTON_H

#include <QToolButton>
#include <QTimer>
/*
 * 使用定时器实现闪烁的效果
 */
class AlarmToolButton : public QToolButton
{
    Q_OBJECT
    // 添加属性，用于QSS选择器
    Q_PROPERTY(AlarmState alarmState READ alarmState WRITE setAlarmState NOTIFY stateChanged)
public:
    enum AlarmState {
        Normal,
        Warning,
        Error
    };
    Q_ENUM(AlarmState)//注册到元对象系统 不能使用enum class 不被 MOC 支持,故不适用LogLevel日志类型
    explicit AlarmToolButton(QWidget *parent = nullptr);
    // 属性访问器
    AlarmState alarmState() const { return m_state; }
    void setAlarmState(AlarmState state);

signals:
    void stateChanged(AlarmState statee);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
private:
    AlarmState m_state{Warning};
    QTimer m_blinkTimer;
    bool m_blinkOn;

private slots:
    void update_blink_state();
};

#endif // ALARMTOOLBUTTON_H
