#include "CameraFocalLengthDialog.h"
#include "ui_CameraFocalLengthDialog.h"

#include "../gui/ChildWindow/ScannerWidget.h"


CameraFocalLengthDialog::CameraFocalLengthDialog(ScannerWidget *parent)
    : scanner(parent)
    , ui(new Ui::CameraFocalLengthDialog)
{
    ui->setupUi(this);
    // 添加单元格变化的信号槽连接
    // connect(ui->tableWidget_CameraFocal, &QTableWidget::cellChanged, this, &CameraFocalLengthDialog::onCellChanged);

    UpdateTableDate();
}

CameraFocalLengthDialog::~CameraFocalLengthDialog()
{
    delete ui;
}

void CameraFocalLengthDialog::UpdateTableDate()
{
    ui->tableWidget_CameraFocal->clearContents();
    ui->tableWidget_CameraFocal->setRowCount(0);
    foreach (auto ref, params) {
        QJsonArray param = ref.toArray();
        int row=ui->tableWidget_CameraFocal->rowCount();
        ui->tableWidget_CameraFocal->insertRow(row);
        ui->tableWidget_CameraFocal->setItem(row,Name,new QTableWidgetItem(param.at(Name).toString()));
        ui->tableWidget_CameraFocal->setItem(row,CameraCenterHight,new QTableWidgetItem(param.at(CameraCenterHight).toString()));
        ui->tableWidget_CameraFocal->setItem(row,ScannerCenterHight,new QTableWidgetItem(param.at(ScannerCenterHight).toString()));
        ui->tableWidget_CameraFocal->setItem(row,Value,new QTableWidgetItem(param.at(Value).toString()));
    }
}

void CameraFocalLengthDialog::onCellChanged(int row, int column)
{
    qDebug() << "Cell changed at row:" << row << "column:" << column;
    QTableWidgetItem* item = ui->tableWidget_CameraFocal->item(row, column);
    if(item) {
        qDebug() << "New value:" << item->text();
    }

    QString str = item->text();
    params[row].toArray()[column] = str;
}

void CameraFocalLengthDialog::on_pushButton_new_info_clicked()
{
    int row = ui->tableWidget_CameraFocal->rowCount();
    ui->tableWidget_CameraFocal->insertRow(row);

    QJsonArray param;
    QString name = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString cameraCenterHight = "1.8";
    QString scannerCenterHight = "0.556";
    param.append(name);
    param.append(cameraCenterHight);
    param.append(scannerCenterHight);
    param.append(QString());
    //添加参数
    ui->tableWidget_CameraFocal->setItem(row,0,new QTableWidgetItem(name));
    ui->tableWidget_CameraFocal->setItem(row,1,new QTableWidgetItem(cameraCenterHight));
    ui->tableWidget_CameraFocal->setItem(row,2,new QTableWidgetItem(scannerCenterHight));
    ui->tableWidget_CameraFocal->setItem(row,3,new QTableWidgetItem(param.at(3).toString()));
    // 添加到数据模型
    params.append(param);
    // 选中新添加的行
    ui->tableWidget_CameraFocal->selectRow(row);

    qDebug() << "Added new row:" << row << "Total rows:" << params.size();
}


void CameraFocalLengthDialog::on_pushButton_start_clicked()
{
    int row = ui->tableWidget_CameraFocal->currentRow();
    if(row == -1){
        ToolTip::ShowText(tr("请先选择要执行的行"), -1);
        return;
    }
    QJsonArray param = params.at(row).toArray();
    QJsonObject res = scanner->parameter;

    res["dir"] = gShare.info.value("dir").toString() + "/config/faro/"+param.at(Name).toString();
    res[Json_NumCols] = 100;

    Session session(scanner->_module(), "SetParameter", res);
    if (gControl.SendAndWaitResult(session,tr("正在设置参数,请等待"))) {
    } else {
        ToolTip::ShowText(tr("设置参数失败"), -1);
        return;
    }

    Session session2(scanner->_module(), "ScanStart");
    if (gControl.SendAndWaitResult(session2,tr("正在启动,请确认等待执行完成后,再执行下一步"))) {
    } else {
        ToolTip::ShowText(tr("启动失败"), -1);
        return;
    }

    Session session3(scanner->_module(), "ScanRecord");
    if (gControl.SendAndWaitResult(session3,tr("开始录制数据,请确认等待执行完成后,再执行下一步"))) {
    } else {
        ToolTip::ShowText(tr("录制失败"), -1);
        return;
    }
}


void CameraFocalLengthDialog::on_pushButton_get_clicked()
{
    int row = ui->tableWidget_CameraFocal->currentRow();
    if(row == -1){
        ToolTip::ShowText(tr("请先选择要执行的行"), -1);
        return;
    }
    QJsonArray param = params.at(row).toArray();
    QJsonObject res;
    // res["dir"] = gShare.info.value("dir").toString() + "/config/faro/"+param.at(Name).toString();
    res["dir"] = "D:/Test/Data";
    res["CameraHight"] = param.at(CameraCenterHight).toString().toDouble();//相机中心到轨面的高度
    res["ScannerHight"] = param.at(ScannerCenterHight).toString().toDouble();//相机中心到轨面的高度
    // res["partes"] = 15;//15机位 15个分组

    Session session(scanner->_module(), "GetCameraPositionDistance",res);
    // gControl.sendTextMessage(session.GetRequest());
    if (gControl.SendAndWaitResult(session,tr("测量相机焦距"),tr("请等待执行完成"),-1)) {
    } else {
    ToolTip::ShowText(tr("测量相机焦距失败"), -1);
        return;
    }
    QString ret = session.result.toString();
    ui->tableWidget_CameraFocal->setItem(row,Value,new QTableWidgetItem(ret));
}


void CameraFocalLengthDialog::on_pushButton_goto_reset_page_clicked()
{
    ui->stackedWidget_action->setCurrentWidget(ui->page_reset);
}


void CameraFocalLengthDialog::on_pushButton_reset_clicked()
{
    int row = ui->tableWidget_CameraFocal->currentRow();
    int col = ui->tableWidget_CameraFocal->currentColumn();
    if(row == -1 || col ==-1){
        ToolTip::ShowText(tr("请先选择要执行的单元格"), -1);
        return;
    }

    QString str = ui->lineEdit_reset->text();
    if(str.isEmpty()){
        ToolTip::ShowText(tr("修改的内容不能为空"), -1);
        return;
    }

    ui->tableWidget_CameraFocal->setItem(row,col,new QTableWidgetItem(str));
    params[row].toArray()[col] = str;
    qDebug() <<"修改的内容不能为"<< params[row].toArray()[col].toString();
}


void CameraFocalLengthDialog::on_pushButton_reset_page_quit_clicked()
{
    ui->stackedWidget_action->setCurrentWidget(ui->page_action);
}

