#include "AcquisitionWizard.h"
#include "ui_AcquisitionWizard.h"

AcquisitionWizard::AcquisitionWizard(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AcquisitionWizard)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
}

AcquisitionWizard::~AcquisitionWizard()
{
    delete ui;
}

void AcquisitionWizard::on_pushButton_Back_clicked()
{
    qDebug()<<"ui->stackedWidget->currentIndex():"<<ui->stackedWidget->currentIndex();
    int index = ui->stackedWidget->currentIndex();
    if(index > 0){
        ui->stackedWidget->setCurrentIndex(index -1);
    }else {
        // ui->pushButton_Back->setEnabled(false);
    }
}


void AcquisitionWizard::on_pushButton_Next_clicked()
{
    qDebug()<<"ui->stackedWidget->currentIndex():"<<ui->stackedWidget->currentIndex();
    int index = ui->stackedWidget->currentIndex();
    if(index < ui->stackedWidget->count() -1){
        ui->stackedWidget->setCurrentIndex(index + 1);
    }else {
        // ui->pushButton_Next->setEnabled(false);
    }
}

