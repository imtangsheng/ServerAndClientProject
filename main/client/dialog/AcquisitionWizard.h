#ifndef ACQUISITIONWIZARD_H
#define ACQUISITIONWIZARD_H

#include <QDialog>

namespace Ui {
class AcquisitionWizard;
}

class AcquisitionWizard : public QDialog
{
    Q_OBJECT

public:
    explicit AcquisitionWizard(QWidget *parent = nullptr);
    ~AcquisitionWizard();

private slots:
    void on_pushButton_Back_clicked();

    void on_pushButton_Next_clicked();

private:
    Ui::AcquisitionWizard *ui;
};

#endif // ACQUISITIONWIZARD_H
