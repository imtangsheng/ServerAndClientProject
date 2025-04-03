// #include "stdafx.h"
#include "CameraWidget.h"
// #include "ui_CameraWidget.h"
CameraWidget::CameraWidget(MainWindow *parent)
    :IWidget(parent)
    , ui(new Ui::CameraWidget)
{
    qDebug()<< "CameraWidget构造函数初始化";
    ui->setupUi(this);
    buttonDeviceManager_ = ui->pushButton_tab_DeviceManager;
    widgetDeviceManager_ = ui->widget_DeviceManager;
    widgetAcquisitionMonitor_ = ui->AcquisitionMonitor;
    IWidget::initialize();
    gSouth.RegisterHandler(sModuleCamera,this);
}

CameraWidget::~CameraWidget()
{
    qDebug() << "CameraWidget 析构函数";
    delete ui;
}

void CameraWidget::initialize()
{
    ui->comboBox_param_keys->clear();
    ui->comboBox_param_keys->addItems(cameraParamsJson.keys());
}

void CameraWidget::prepare()
{

}

void CameraWidget::start()
{

}

void CameraWidget::stop()
{

}

void CameraWidget::ShowMessage(const QString &msg)
{
    qDebug() << "CameraWidget::onShow(const QString &"<<msg;

    // ui->label_TitleAdditionalInformation
}

void CameraWidget::updata_params(QJsonObject params)
{
    cameraParamsJson = params;
    initialize();
}

void CameraWidget::on_pushButton_test_clicked()
{
    gClient.sendTextMessage(Session::RequestString(11,sModuleCamera,"test","测试参数"));
}


void CameraWidget::retranslate_ui()
{
    qDebug() << "void CameraWidget::retranslate_ui()";
    ui->retranslateUi(this);
}

void CameraWidget::on_pushButton_scan_clicked()
{
    // gClient.sendTextMessage(Session::RequestString(11,sModuleCamera,"scan",true));
    Session session({ {"id", 11}, {"module", sModuleCamera}, {"method", "scan"}, {"params", true} });
    session.socket = &gClient;
    WaitDialog wait(this,&session,20);// = new WaitDialog(this);
    //100ms 以内防止弹窗显示
    if(wait.init() || wait.exec() == QDialog::Accepted){
        qDebug() << " QDialog::Accepted";
    }else{
        qDebug() << "QDialog::Rejected"<<QDialog::Rejected;
        return;
    }
    qDebug() << session.result;
    QString names = session.result.toString();
    qDebug() << "Result Camera::scan()"<<names;
    QStringList cameralist = names.split(",");
    qDebug() << cameralist;
    ui->comboBox_device_list->clear();
    ui->comboBox_device_list->addItems(cameralist);
}


void CameraWidget::on_pushButton_open_clicked()
{
    Session session({ {"id", 11}, {"module", sModuleCamera}, {"method", "open"}, {"params", true} });
    gController.handleSession(session);
}


void CameraWidget::on_pushButton_start_clicked()
{
    Session session({ {"id", 11}, {"module", sModuleCamera}, {"method", "start"}, {"params", QJsonArray()} });
    gController.handleSession(session);
}


void CameraWidget::on_pushButton_stop_clicked()
{
    Session session({ {"id", 11}, {"module", sModuleCamera}, {"method", "stop"}, {"params", QJsonArray()} });
    gController.handleSession(session);
}


void CameraWidget::on_pushButton_showProperty_clicked()
{
    //ToDo 显示相机的属性控制界面
    gClient.sendTextMessage(Session::RequestString(11,sModuleCamera,"show",true));
}


void CameraWidget::on_pushButton_param_setting_clicked()
{
    Session session({ {"id", 11}, {"module", sModuleCamera}, {"method", "SetCamerasParams"}, {"params", cameraParamsJson} });
    gController.handleSession(session);
}


void CameraWidget::on_pushButton_param_add_key_clicked()
{
    QJsonObject param;
    QString key = ui->lineEdit_param_key->text();
    param["NodeName"] = key;

    QString type= ui->comboBox_param_type->currentText();
    param["Type"] = type;
    QString defaultValue = ui->lineEdit_param_value->text();// 根据类型转换默认值
    if (type == "enum") {
        param["Default"] = defaultValue.toInt();
    } else if (type == "bool") {
        param["Default"] = defaultValue.toLower() == "true";
    } else if (type == "float") {
        param["Default"] = defaultValue.toDouble();
    } else if (type == "int32") {
        param["Default"] = defaultValue.toInt();
    } else {
        // "string"
        param["Default"] = defaultValue;
    }
    param["Description"] = ui->lineEdit_param_description->text();
    cameraParamsJson[key] = param;
    ui->comboBox_param_keys->clear();
    ui->comboBox_param_keys->addItems(cameraParamsJson.keys());
}


void CameraWidget::on_pushButton_param_delete_key_clicked()
{
    QString key = ui->lineEdit_param_key->text();
    cameraParamsJson.remove(key);

    ui->comboBox_param_keys->clear();
    ui->comboBox_param_keys->addItems(cameraParamsJson.keys());
}

void CameraWidget::on_pushButton_param_server_get_clicked()
{
    gClient.sendTextMessage(Session::RequestString(11,sModuleCamera,"onParamsChanged",QJsonArray()));
}

void CameraWidget::on_pushButton_param_server_save_clicked()
{
    Session session({ {"id", 11}, {"module", sModuleCamera}, {"method", "SaveCamerasParams"}, {"params", cameraParamsJson} });
    gController.handleSession(session);

}


void CameraWidget::on_comboBox_param_keys_currentTextChanged(const QString &arg1)
{
    QJsonObject param = cameraParamsJson[arg1].toObject();
    ui->lineEdit_param_key->setText(param["NodeName"].toString());
    QString type = param["Type"].toString();
    ui->comboBox_param_type->setCurrentText(type);
    QString value;
    if (type == "enum" || type == "int32") {
        value = (QString::number(param["Default"].toInt()));
    }
    else if (type == "bool") {
        value = (param["Default"].toBool() ? "true" : "false");
    }
    else if (type == "float") {
        // 使用 QString::number 来格式化浮点数，避免科学计数法
        value = (QString::number(param["Default"].toDouble(), 'f', 6));
    }
    else {
        // "string"
        value = (param["Default"].toString());
    }

    ui->lineEdit_param_value->setText(value);
    ui->lineEdit_param_description->setText( param["Description"].toString());

}


void CameraWidget::on_pushButton_trigger_clicked()
{
    Session session({ {"id", 11}, {"module", sModuleCamera}, {"method", "trigger"}, {"params", true} });
    gController.handleSession(session);
}


void CameraWidget::on_pushButton_frame_update_clicked()
{
    gClient.sendTextMessage(Session::RequestString(11,sModuleCamera,"GetUpdateFrameInfo",""));
}




