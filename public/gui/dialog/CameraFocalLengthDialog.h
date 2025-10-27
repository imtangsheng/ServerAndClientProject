#ifndef CAMERAFOCALLENGTHDIALOG_H
#define CAMERAFOCALLENGTHDIALOG_H


#include <QDialog>
#include <QJsonArray>
class ScannerWidget;
namespace Ui {
class CameraFocalLengthDialog;
}

class CameraFocalLengthDialog : public QDialog
{
    Q_OBJECT

    enum ColumnIndex {
        Name = 0,
        CameraCenterHight = 1,
        ScannerCenterHight = 2,
        Value = 3
    };
public:
    explicit CameraFocalLengthDialog(ScannerWidget *parent = nullptr);
    ~CameraFocalLengthDialog();
    QJsonArray params;
    void UpdateTableDate();

private slots:
    void on_pushButton_new_info_clicked();

    void on_pushButton_start_clicked();

    void on_pushButton_get_clicked();

    void on_pushButton_goto_reset_page_clicked();

    void on_pushButton_reset_clicked();

    void on_pushButton_reset_page_quit_clicked();

    void on_tableWidget_CameraFocal_cellChanged(int row, int column);

private:
    ScannerWidget* scanner;
    Ui::CameraFocalLengthDialog *ui;

};

#endif // CAMERAFOCALLENGTHDIALOG_H
