#ifndef PROJECTITEMRADIOBUTTON_H
#define PROJECTITEMRADIOBUTTON_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class ProjectItemRadioButton;
}

class ProjectItemRadioButton : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectItemRadioButton(const QJsonObject& project,QWidget *parent = nullptr);
    ~ProjectItemRadioButton();
    QString text;

    void SetChecked(bool checked);

private:
    Ui::ProjectItemRadioButton *ui;
    QJsonObject m_project;

signals:
    void clicked(const QJsonObject& project);
protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            SetChecked(true);
            emit clicked(m_project);
        }
    }
};

#endif // PROJECTITEMRADIOBUTTON_H
