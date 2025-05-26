#ifndef CREATENEWPROJECT_H
#define CREATENEWPROJECT_H

#include <QDialog>

namespace Ui {
class CreateNewProject;
}

class CreateNewProject : public QDialog
{
    Q_OBJECT

public:
    explicit CreateNewProject(QWidget *parent = nullptr);
    ~CreateNewProject();
    FileInfoDetails project{};
private slots:
    void on_pushButton_Accepted_clicked();

private:
    Ui::CreateNewProject *ui;
};

#endif // CREATENEWPROJECT_H
