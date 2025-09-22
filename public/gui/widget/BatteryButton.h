#ifndef BATTERYBUTTON_H
#define BATTERYBUTTON_H

#include <QPushButton>
#include <QTimer>
class BatteryButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(Level level READ getLevel WRITE setLevel NOTIFY levelChanged)
public:
    enum Level:quint8 {
        Unknown=0,
        VeryLow,    // 0-9%
        Low,      // 10-29%
        Medium,   // 30-59%
        High,     // 60-79%
        Full      // 80-100%
    };
    Q_ENUM(Level)//注册到元对象系统 不能使用 enum class 不被 MOC 支持
    explicit BatteryButton(QWidget *parent = nullptr);
    Level getLevel(){return m_level;}
    void setLevel(Level level);
    void setActive(bool active);
    void setValue(quint8 value,double voltage);

protected:
    void paintEvent(QPaintEvent* event) override;
private:
    Level m_level{ Unknown };
    bool m_active{ false };
    QTimer m_blinkTimer;
    bool m_blinkOn;
signals:
    void levelChanged();
};

#endif // BATTERYBUTTON_H
