// #include "stdafx.h"
#include "CameraWidget.h"
#include "mainwindow.h"

CameraWidget::CameraWidget(MainWindow *parent)
    :ChildWidget(parent)
    , ui(new Ui::CameraWidget)
{
    qDebug()<< "CameraWidget构造函数初始化";
    ui->setupUi(this);
    ui->comboBox_param_templates->setModel(&parent->paramNamesModel);
    deviceType = Camera;
    ChildWidget::initialize();
    gShare.RegisterHandler(_module(),this);// 注册当前类的成员函数作为处理器

    //直接函数指针（仅限静态成员函数）
    //
    // 使用std::bind将成员函数绑定到当前对象  需要显式指定占位符 _1
    //gShare.handlerBinarySession[share::ModuleName::camera] = std::bind(&CameraPlugin::handle_binary_message, this, std::placeholders::_1);
    // 注册当前类的成员函数作为处理器
    gShare.handlerBinarySession[share::ModuleName::camera] = [this](const QByteArray& bytes) {
        qDebug() <<"gShare.handlerBinarySession[share::ModuleName::camera] = [this](const QByteArray& bytes"<< bytes.size();
        handle_binary_message(bytes);
    };

    // 在构造函数或初始化时启用双缓冲
    ui->label_image_show->setAttribute(Qt::WA_OpaquePaintEvent, true);
    ui->label_image_show->setAttribute(Qt::WA_NoSystemBackground, true);
    ui->label_image_show->setAutoFillBackground(false);
    qDebug()<<"使用 QVariant 获取枚举值对应的字符串:"<<QVariant::fromValue(deviceType).toString();
    // ui->comboBox_param_type->addItems("enum,float,int32,bool,string");

    //初始获取图片格式指令 ,需要缓存
    // SentMessageToClients(Session::RequestString(1, sModuleUser, "login_new_session", QJsonArray{ double(deviceType)}));
}

CameraWidget::~CameraWidget()
{
    qDebug() << "CameraWidget 析构函数";
    delete ui;
}

void CameraWidget::initialize()
{

}

QString CameraWidget::_module() const
{
    static QString module = share::Shared::GetModuleName(share::ModuleName::camera);
    return module;
}

void CameraWidget::ShowMessage(const QString &msg)
{
    qDebug() << "CameraWidget::onShow(const QString &"<<msg;

    // ui->label_TitleAdditionalInformation
}

void CameraWidget::handle_binary_message(const QByteArray &bytes)
{
    qDebug() <<"CameraWidget::handle_binary_message(const QByteArray &data)"<<bytes.size();
    QDataStream readStream(bytes);
    quint8 invoke;
    quint8 userID;// = QString(info.UserID).toInt();
    quint32  triggerID;// = pFrame->uTriggerId;
    readStream >>invoke>> userID >> triggerID;
    QByteArray imageBytes;
    readStream >> imageBytes;
    // QSize size;
    // QImage::Format format;
    // readStream >> size >> format;
    // QImage newImage(size, format);
    // readStream.readRawData(reinterpret_cast<char*>(newImage.bits()), newImage.sizeInBytes());
    ui->label_image_title->setText(tr("%1-%2").arg(userID).arg(triggerID));
    // ui->label_image_show->setPixmap(QPixmap::fromImage(newImage));
    ui->label_image_show->setPixmap(QPixmap::fromImage(QImage::fromData(imageBytes)));

}

void CameraWidget::initUi(const Session &session)
{
    QJsonObject obj = session.result.toObject();
    if(obj.isEmpty()) return;
    isInitUi = true;
    if(obj.contains("format")){
        ui->comboBox_image_format->clear();
        ui->comboBox_image_format->addItems(obj.value("format").toString().split(","));
    }
}

void CameraWidget::onDeviceStateChanged(double state, QString message)
{
    //默认触发就是在线状态,返回值为false
    if(!isInitUi){
        gControl.sendTextMessage(Session::RequestString(11,_module(),"initUi",state));
    }
    return ChildWidget::onDeviceStateChanged(state,message);
}

void CameraWidget::onConfigChanged(QJsonObject config)
{
    config_ = config;
    parameter = config_.value("params").toObject();
    task = config_.value("task").toObject();
    general = config_.value("general").toObject();
    //设备配置参数
    {
    QSignalBlocker blocker(ui->comboBox_param_keys);// 阻塞信号避免触发变化
    ui->comboBox_param_keys->clear();//值为空,会触发一次文本变化信号
    }
    ui->comboBox_param_keys->addItems(parameter.keys());

    //任务参数 ,所有配置过的设备id
    {
    QSignalBlocker blocker2(ui->comboBox_device_list);// 阻塞信号避免触发变化
    ui->comboBox_device_list->clear();
    }
    ui->comboBox_device_list->addItems(task.keys());

    //相机照片保存格式
    QString format = general.value("format").toString();
    if(!format.isEmpty())
        ui->comboBox_image_format->setCurrentText(format);
}

void CameraWidget::onImageInfoChanged(const Session &session)
{
    //Todo
    QJsonObject imageInfo = session.params.toObject();
    QString id = imageInfo.value("id").toString();
    qint64 frameId = imageInfo.value("frameId").toInt(-1);
    QString base64Data = imageInfo.value("data").toString();

    qDebug() <<"onImageInfoChanged: id frameId"<< id<<frameId <<"base64Data:"<<base64Data.size();
    ui->label_image_title->setText(tr("%1-%2").arg(id).arg(frameId));
    if(base64Data.isEmpty()){
        qWarning() << "Empty image data from camera:" << id << frameId;
        return;
    }
    // 3. Base64解码
    QByteArray imageBytes = QByteArray::fromBase64(base64Data.toLatin1());
    if (imageBytes.isEmpty()) {
        qWarning() << "Base64 decode failed for frame:" << frameId;
        return;
    }
    // // 4. 加载图像
    // QImage image;
    // if (!image.loadFromData(imageBytes)) {
    //     // 尝试通过格式推断加载
    //     QString formatHint = imageInfo["format"].toString("JPEG"); // 默认为JPEG
    //     if (!image.loadFromData(imageBytes, formatHint.toUpper().toLatin1())) {
    //         qCritical() << "Image load failed, frame:" << frameId
    //                     << "size:" << imageBytes.size() << "bytes";
    //         return;
    //     }
    // }
    qDebug() << "image bytes:"<<imageBytes.size();
    ui->label_image_show->setPixmap(QPixmap::fromImage(QImage::fromData(imageBytes)));
    // ui->toolButton_image->setIcon(QIcon(QPixmap::fromImage(QImage::fromData(imageBytes))));
}

QRadioButton *CameraWidget::GetButtonDeviceManager()
{
    return ui->ButtonMenu_DeviceManager;
}

QWidget *CameraWidget::GetWidgetDeviceManager()
{
    return ui->widget_DeviceManager;
}

QWidget *CameraWidget::GetWidgetAcquisitionMonitor()
{
    return ui->AcquisitionMonitor;
}

void CameraWidget::retranslate_ui()
{
    qDebug() << "void CameraWidget::retranslate_ui()";
    ui->retranslateUi(this);
}

// void CameraWidget::on_pushButton_test_clicked()
// {
//     gControl.sendTextMessage(Session::RequestString(11,_module(),"test","测试参数"));
// }

void CameraWidget::on_comboBox_device_list_currentTextChanged(const QString &arg1)
{
    qDebug() << "CameraWidget::on_comboBox_device_list_currentTextChanged(const QString &"<<arg1;
    if(arg1.isEmpty()) return;
    QJsonObject device = task.value(arg1).toObject();
    ui->lineEdit_file_path->setText(device["path"].toString());

}

void CameraWidget::on_pushButton_scan_clicked()
{
    // gControl.sendTextMessage(Session::RequestString(11,_module(),"scan",true));
    Session session({ {"id", 11}, {"module", _module()}, {"method", "scan"}, {"params", true} });
    if(!gControl.SendAndWaitResult(session)){
        qDebug() << "QDialog::Rejected"<<QDialog::Rejected;
        return;
    }
    qDebug() << session.result;
    QJsonObject obj = session.result.toObject();
    QString names = obj.value("camera_id_list").toString();
    qDebug() << "Result Camera::scan()"<<names;
    QStringList cameralist = names.split(",");
    qDebug() << cameralist;
    {
        QSignalBlocker blocker(ui->comboBox_device_list);// 阻塞信号避免触发变化
        ui->comboBox_device_list->clear();
    }
    ui->comboBox_device_list->addItems(cameralist);

    //MS301 的有雷达串口
    QString serialPortNames = obj.value("serial_id_list").toString();
    QStringList serialPortNameList = serialPortNames.split(",");
    qDebug() << "serialPortNameList "<<serialPortNameList ;
    ui->comboBox_serial_names->addItems(serialPortNameList);

}

void CameraWidget::on_pushButton_task_key_reset_clicked()
{
    QJsonObject key;
    key["path"] = ui->lineEdit_file_path->text();
    task[ui->comboBox_device_list->currentText()] = key;
}

void CameraWidget::on_comboBox_param_keys_currentTextChanged(const QString &arg1)
{
    qDebug() <<"CameraWidget::on_comboBox_param_keys_currentTextChanged(const QString &"<<arg1;
    if(arg1.isEmpty()) return;
    // parameter[arg1].toObject();会修改对象Qt的QJsonObject设计导致的，使用[]操作符是具有"插入"功能的
    QJsonObject param = parameter.value(arg1).toObject();
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
    else if(type == "string"){
        // "string"
        value = (param["Default"].toString());
    }else{
        qWarning() << "添加了不被支持的类型"<<type;
    }

    ui->lineEdit_param_value->setText(value);
    ui->lineEdit_param_description->setText( param["Description"].toString());

}


void CameraWidget::on_pushButton_param_setting_clicked()
{
    Session session({ {"id", 11}, {"module", _module()}, {"method", "SetCamerasParams"},
         {"params",config_ }});
    gControl.SendAndWaitResult(session);
}


void CameraWidget::on_pushButton_param_add_key_clicked()
{
    QString key = ui->lineEdit_param_key->text();
    if(key.isEmpty()) return;
    QJsonObject param;
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
    } else if(type == "string"){
        // "string"
        param["Default"] = defaultValue;
    } else{
        qWarning() << "添加了不被支持的类型"<<type;
    }
    param["Description"] = ui->lineEdit_param_description->text();
    parameter[key] = param;
    {
    QSignalBlocker blocker(ui->comboBox_param_keys);// 阻塞信号避免触发变化
    // ui->comboBox_param_keys->addItem(key);
    ui->comboBox_param_keys->clear();
    ui->comboBox_param_keys->addItems(parameter.keys());
    }
    ui->comboBox_param_keys->setCurrentText(key);
}


void CameraWidget::on_pushButton_param_delete_key_clicked()
{
    QString key = ui->lineEdit_param_key->text();
    parameter.remove(key);
    {
    QSignalBlocker blocker(ui->comboBox_param_keys);// 阻塞信号避免触发变化
    ui->comboBox_param_keys->clear();
    }
    ui->comboBox_param_keys->addItems(parameter.keys());
}

void CameraWidget::on_pushButton_param_server_get_clicked()
{
    gControl.sendTextMessage(Session::RequestString(11,_module(),"onConfigChanged",QJsonArray()));
}

void CameraWidget::on_pushButton_param_server_save_clicked()
{
    config_["version"] = "0.0.1";
    config_["lastUpdate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    config_["task"] = task;
    config_["params"] = parameter;
    config_["general"] = general;
    Session session({ {"id", 11}, {"module", _module()}, {"method", "SaveConfig"}, {"params", config_} });
    gControl.SendAndWaitResult(session);

}


void CameraWidget::on_pushButton_trigger_clicked()
{
    Session session({ {"id", 11}, {"module", _module()}, {"method", "trigger"}, {"params", true} });
    gControl.SendAndWaitResult(session);
}


// void CameraWidget::on_pushButton_frame_update_clicked()
// {
//     gControl.sendTextMessage(Session::RequestString(11,_module(),"GetUpdateFrameInfo",true));
// }


void CameraWidget::on_pushButton_open_clicked()
{
    Session session({ {"id", 11}, {"module", _module()}, {"method", "open"}, {"params", true} });
    gControl.SendAndWaitResult(session);
}


void CameraWidget::on_pushButton_start_clicked()
{
    Session session({ {"id", 11}, {"module", _module()}, {"method", "start"}, {"params", true} });
    gControl.SendAndWaitResult(session);
}


// void CameraWidget::on_pushButton_stop_clicked()
// {
//     Session session({ {"id", 11}, {"module", _module()}, {"method", "stop"}, {"params", true} });
//     gControl.SendAndWaitResult(session);
// }


// void CameraWidget::on_pushButton_showProperty_clicked()
// {
//     //ToDo 显示相机的属性控制界面
//     gControl.sendTextMessage(Session::RequestString(11,_module(),"show",true));
// }

void CameraWidget::on_pushButton_image_format_set_clicked()
{
    //相机照片保存格式
    QString format = ui->comboBox_image_format->currentText();
    general["format"] = format;
    Session session({ {"id", 11}, {"module", _module()}, {"method", "SetImageFormat"}, {"params", format} });
    gControl.SendAndWaitResult(session);
}


void CameraWidget::on_toolButton_image_clicked()
{
    qDebug() <<"CameraWidget::on_toolButton_image_clicked()";
}


void CameraWidget::on_pushButton_serial_names_set_clicked()
{
    QString serialName = ui->comboBox_serial_names->currentText();
    if(serialName.isEmpty()){
        qWarning() << "ui->comboBox_serial_names->currentText(); is Enpty";
        return;
    }
    QJsonObject obj;
    obj[CAMERA_KEY_PORTNAME] = serialName;
    Session session({ {"id", 11}, {"module", sModuleUser}, {"method", "SetRegisterSettings"}, {"params", obj} });
    gControl.SendAndWaitResult(session);
}

