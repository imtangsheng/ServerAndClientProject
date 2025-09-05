#ifndef PROJECTITEMCHECKBOX_H
#define PROJECTITEMCHECKBOX_H

#include <QWidget>
#include <QMouseEvent>

namespace Ui {
class ProjectItemCheckBox;
}

class ProjectItemCheckBox : public QWidget
{
    Q_OBJECT

public:
    explicit ProjectItemCheckBox(const FileInfoDetails& project,QWidget *parent = nullptr);
    ~ProjectItemCheckBox();
    FileInfoDetails project;
    // QString text;
    bool isChecked();
    void SetChecked(bool checked);
    void onCurrentProjectChanged();
private:
    Ui::ProjectItemCheckBox *ui;

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        SetChecked(true);
    }
    void mouseDoubleClickEvent(QMouseEvent *event) final{
        onCurrentProjectChanged();
        SetChecked(true);
    }
};

#endif // PROJECTITEMCHECKBOX_H
