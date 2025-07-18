#include "MainWindow.h"

#include "ui/AboutDialog.h"
#include "ui/WebSocketWidget.h"
QPointer<WebSocketWidget>gWebWidget;


#include"ChildWindow/TrolleyWidget.h"
#include"ChildWindow/CameraWidget.h"
#include"ChildWindow/ScannerWidget.h"

QPointer<TrolleyWidget>gTrolley;
#define SAFE_TROLLEY_CALL(expr) \
    ((gTrolley) ? (gTrolley->expr) : false)

QPointer<CameraWidget>gCamera;
QPointer<ScannerWidget>gScanner;

QDateTime gSearchStartDateTime;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);//无边框和标题栏
    MainBackgroundWidget = ui.MainStackedWidget;
    //连接翻译
    connect(this,&MainWindow::languageChanged,this,&MainWindow::_retranslate);
    //网络设置
    gWebWidget = new WebSocketWidget(&gClient);
    connect(gWebWidget->socket, &QWebSocket::connected, this, &MainWindow::onConnectSocket);
    connect(gWebWidget->socket, &QWebSocket::disconnected, this, &MainWindow::onDisconnectSocket);
    connect(gWebWidget->socket,&QWebSocket::errorOccurred,this, &MainWindow::onSocketError);

    //系统设置界面
#pragma region settings系统设置数据界面
    //#网络设置
    gWebWidget->url = gSettings->value("network/url", "ws://localhost:8080").toString();
    ui.lineEdit_network_url->setText(gWebWidget->url);
    gWebWidget->isAutoReconnect = gSettings->value("network/AutoReconnect", true).toBool();
    ui.radioButton_network_reconnect_on->setChecked(gWebWidget->isAutoReconnect);
    gWebWidget->reconnectInterval = gSettings->value("network/ReconnectInterval", 5000).toInt();
    ui.spinBox_network_reconnect_interval->setValue(gWebWidget->reconnectInterval);
    //#设备启用 /*界面控件显示 加载模块*/
    if (gSettings->value("device/trolley", true).toBool()) gTrolley = new TrolleyWidget(this);
    if (gSettings->value("device/camera", true).toBool()) gCamera = new CameraWidget(this);
    if (gSettings->value("device/scanner", true).toBool()) gScanner = new ScannerWidget(this);
    //#默认中文界面 如果为英语,首先需要触发一次,所以使用改变信号
    if (gSettings->value("language",zh_CN).toString() == en_US) {
        ui.radioButton_language_en_US->setChecked(true);
    }
    //#启动设置 默认已经开机自启动 最大化
    if (!gSettings->value("AutoStartup", true).toBool()) {
        ui.radioButton_startup_auto_off->setChecked(true);
    }
    if (gSettings->value("IsFullScreen", false).toBool()) {
        ui.radioButton_startup_fullscreen_on->setChecked(true);
        //检查是否全屏
        if(isMaximized()){
            ui.toolButton_maximize->setVisible(false);
        }else{
            showFullScreen();
        }
    } else {
        showNormal();
        ui.toolButton_normal->setVisible(false);
    }

    //#主题设置 默认深色 浅色未适配
    if (!gSettings->value("ThemeDark", true).toBool()) {
        ui.radioButton_theme_light->setChecked(true);
    }
    //#日志设置
    if (!gSettings->value("CarWarning", true).toBool()) {
        ui.radioButton_car_warning_off->setChecked(true);
    }
    gLog.logLevel = static_cast<LogLevel>(gSettings->value("LogLevel", +LogLevel::Warning).toInt());
    if(gLog.logLevel != LogLevel::Warning){
        if(gLog.logLevel == LogLevel::Debug) ui.radioButton_log_debug->setChecked(true);
        else if(gLog.logLevel == LogLevel::Info) ui.radioButton_log_info->setChecked(true);
        else if(gLog.logLevel == LogLevel::Error) ui.radioButton_log_error->setChecked(true);
    }
#pragma endregion

#pragma region PageProjectHub项目页
    //界面设置初始化
    ui.ProjectsWidgetContents->setContentsMargins(30, 30, 30, 30);
    // 设置列拉伸
    ui.ProjectsWidgetContents->setColumnStretch(0, 1);
    ui.ProjectsWidgetContents->setColumnStretch(1, 1);
    ui.ProjectsWidgetContents->setColumnStretch(2, 1);

    connect(&gControl, &CoreControl::onProjectClicked, this, &MainWindow::onEnterProjectClicked);

// #PageTask任务页
    UpdateCitySubwayInfo(":assets/docs/city");
#pragma endregion
#pragma region PageTemplate参数模板页
    ui.LayoutParamTemplate->setColumnStretch(0, 1);
    ui.LayoutParamTemplate->setColumnStretch(1, 1);
    connect(&gControl, &CoreControl::onParamTemplateClicked, this, &MainWindow::CurrentParamTemplateChanged);
    UpdateLayoutParamTemplate();
#pragma endregion

}

MainWindow::~MainWindow() {
    delete gTrolley;
    delete gCamera;
    delete gScanner;
}

#include"button/ProjectItemCheckBox.h"
void MainWindow::AddProjectWidget(FileInfoDetails project) {
    ProjectItemCheckBox* button_project = new ProjectItemCheckBox(project, ui.widget_projects_contents);
    ui.ProjectsWidgetContents->addWidget(button_project);
    this->projectItemWidget.append(button_project);
}

void MainWindow::UpdateLayoutProjectWidget() {
    //for (auto* widget : projectItemWidget){
    //    if (widget) ui.ProjectsWidgetContents->removeWidget(widget);
    //}
    // 清除现有项目
    while (QLayoutItem* item = ui.ProjectsWidgetContents->takeAt(0)){
        ui.ProjectsWidgetContents->removeItem(item);
    }
    foreach (auto* button , projectItemWidget){
        ui.ProjectsWidgetContents->addWidget(button);
    }
}

bool MainWindow::UpdateProjects() {
    Session session(sModuleUser, "GetTaskData", "");//关机函数执行
    if(!gControl.SendAndWaitResult(session)){
        return false;
    }
    QJsonObject obj = session.result.toObject();
    if(obj.isEmpty()) return false;
    gTaskManager.data = obj;
    QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
    foreach(const QString& key, projects.keys()){
        this->AddProjectWidget(projects.value(key).toObject());
    }
    return true;
}

void MainWindow::SetLogLevel(LogLevel level)
{
    Session session(sModuleUser,"onLogLevelChanged",+level);
    if(gControl.SendAndWaitResult(session)){
        gLog.logLevel =level;
        gSettings->setValue("LogLevel",+level);
    }
}

void MainWindow::GotoHomePage() {
    qDebug() << "#Window::GotoHomePage()";
    ui.MainStackedWidget->setCurrentWidget(ui.PageHome);
}

void MainWindow::onEnterProjectClicked(const FileInfoDetails &project) {
    qDebug() << "#Window::onEnterProjectClicked(QJsonObject project)"<<project.ToJsonObject();
    ui.StackedWidgetProjectHub->setCurrentWidget(ui.TaskHome);
    if(ui.MainStackedWidget->currentWidget() != ui.PageProjectHub){
        ui.MainStackedWidget->setCurrentWidget(ui.PageProjectHub);
    }
    ui.label_current_task_name->hide();
    ui.label_current_project_name->setText(tr("当前项目:%1").arg(project.name));
    ui.lineEdit_task_param_content_name->setText(QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss"));

    static bool needUpdateCameraFormat{true};
    if(needUpdateCameraFormat){
        Session session(sModuleCamera, "GetCameraFormat", "");
        if(gControl.SendAndWaitResult(session)){
            UpdateCameraFormat(session.result.toString());
            needUpdateCameraFormat = false;
        }else{
            ToolTip::ShowText(tr("获取相机照片格式失败"),-1);
        }
    }
}

void MainWindow::onShowMessage(const QString& message, LogLevel level) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString levelStr;
    QColor color;
    switch (level) {
    case LogLevel::Error:
        levelStr = tr("[Error]");
        color = Qt::red;
        ui.toolButton_alarm->setAlarmState(AlarmToolButton::Error);
        break;
    case LogLevel::Warning:
        levelStr = tr("[Warning]");
        color = Qt::yellow;
        ui.toolButton_alarm->setAlarmState(AlarmToolButton::Warning);
        break;
    default:
        color = QColor(128, 128, 128);// 灰色
        break;
    }
    qDebug() << int(level) << message << color.name();
    // ui->plainTextEdit_log->appendPlainText(QString("[%1] %2 %3").arg(timestamp).arg(levelStr).arg(message));
    ui.plainTextEdit_log->appendHtml(QString("<span style='color: %1'>[%2] %3 %4</span>").arg(color.name(), timestamp, levelStr, message));

}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        ui.toolButton_maximize->setVisible(!isMaximized());
        ui.toolButton_normal->setVisible(isMaximized());
    }
    return QMainWindow::changeEvent(event);
}

void MainWindow::onConnectSocket() {
    gWebWidget->socket->sendTextMessage(Session::RequestString(Session::NextId(),sModuleUser,"login",gShare.sessiontype_));
    gWebWidget->reconnectTimer.stop();

    ui.pushButton_network_connect->setText(tr("断开"));
    gControl.sockets.insert(gWebWidget->socket);//添加到全局
}

void MainWindow::onDisconnectSocket() {
    ui.pushButton_network_connect->setText(tr("连接"));

    if(gWebWidget->isAutoReconnect){
        gWebWidget->reconnectTimer.start(gWebWidget->reconnectInterval);
    }

    gControl.sockets.remove(gWebWidget->socket);
}

void MainWindow::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorString;
    // Q_UNUSED(error)
    switch (error) {
    case QAbstractSocket::ConnectionRefusedError:
        errorString = tr("连接被拒绝");
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorString = tr("远程主机关闭连接");
        break;
    case QAbstractSocket::HostNotFoundError:
        errorString = tr("主机未找到");
        break;
    case QAbstractSocket::SocketTimeoutError:
        errorString = tr("连接超时");
        break;
    default:
        errorString = tr("网络错误: %1").arg(gWebWidget->socket->errorString());
    }
    ToolTip::ShowText(tr("网络错误"),errorString);
    ui.pushButton_network_connect->setText(tr("连接"));
}

void MainWindow::on_action_goto_home_triggered() {
    GotoHomePage();
}

void MainWindow::on_action_start_triggered()
{
    qDebug() << "#Window::on_action_start_triggered()";
}

void MainWindow::on_action_stop_triggered()
{
    qDebug() << "#Window::on_action_stop_triggered()";
}


void MainWindow::on_pushButton_project_hub_clicked() {
    ui.MainStackedWidget->setCurrentWidget(ui.PageProjectHub);
}


void MainWindow::on_pushButton_system_settings_clicked() {
    ui.MainStackedWidget->setCurrentWidget(ui.PageSystem);
}


void MainWindow::on_pushButton_device_management_clicked() {
    ui.MainStackedWidget->setCurrentWidget(ui.PageDeviceManagement);
}


void MainWindow::on_pushButton_parameter_templates_clicked() {
    static bool isFirst = true;
    if(isFirst){
        UpdateLayoutParamTemplate();
        isFirst = false;
    }
    ui.MainStackedWidget->setCurrentWidget(ui.PageTemplates);
}


void MainWindow::on_pushButton_data_copy_clicked() {
    ui.MainStackedWidget->setCurrentWidget(ui.PageDataHub);
}


void MainWindow::on_pushButton_project_start_clicked() {
    ui.MainStackedWidget->setCurrentWidget(ui.PageProjectHub);
    static bool isNeedUpdate = true;
    if(isNeedUpdate && UpdateProjects()){
        isNeedUpdate = false;
    }
}


void MainWindow::on_toolButton_alarm_clicked() {
    ui.MainStackedWidget->setCurrentWidget(ui.PageLog);
}


void MainWindow::on_pushButton_shutdown_clicked() {
    Session session(sModuleUser, "shutdown", QJsonArray());//关机函数执行
    gControl.SendAndWaitResult(session);
}

#pragma region page_home系统设置页面
// clicked() 信号先触发
// toggled() 信号后触发
//用于信号设置,只要变化都会触发
void MainWindow::on_radioButton_language_en_US_clicked() {
    qDebug() << "MainWindow::on_radioButton_language_en_US_clicked()";
    gSettings->setValue("language", en_US);
}

void MainWindow::on_radioButton_language_en_US_toggled(bool checked) {
    qDebug() << "MainWindow::on_radioButton_language_en_US_toggled(bool checked)" << checked;
    if (checked) {
        if (g_translator.load(QString(":/i18n/%1").arg(en_US))) {
            qApp->installTranslator(&g_translator);
        } else {
            onShowMessage(tr("fail language switch:%1").arg(en_US), LogLevel::Warning);
        }
    } else {
        qApp->removeTranslator(&g_translator);
        gSettings->setValue("language", zh_CN);
    }
    gControl.sendTextMessage(Session::RequestString(Session::NextId(),sModuleUser,"onLanguageChanged",QJsonArray{checked ? en_US : zh_CN}));
    emit languageChanged();
}

void MainWindow::on_pushButton_network_connect_clicked() {
    if(gWebWidget->socket->state() == QAbstractSocket::UnconnectedState){
        QString urlStr = ui.lineEdit_network_url->text().trimmed();// 获取URL并连接
        QUrl url(urlStr);
        if (!url.isValid()) {
            ToolTip::ShowText(tr("网络错误"),tr("无效的url地址!请重新输入"));
            return;
        }
        gWebWidget->socket->open(url);
        qDebug() << "Connecting to " << url;
        ui.pushButton_network_connect->setText(tr("连接中"));
    }else{
        gWebWidget->socket->close();
        ui.pushButton_network_connect->setText(tr("连接"));
    }
}

void MainWindow::on_radioButton_startup_auto_off_clicked() {
    gSettings->setValue("AutoStartup", false);
}

void MainWindow::on_radioButton_startup_auto_off_toggled(bool checked) {
    if(!checked) gSettings->setValue("AutoStartup", true);
    Session session(sModuleUser,"onAutoStartedClicked",checked);
    gControl.SendAndWaitResult(session);
}

void MainWindow::on_radioButton_startup_fullscreen_on_clicked()
{
    gSettings->setValue("IsFullScreen", true);
}

void MainWindow::on_radioButton_startup_fullscreen_off_clicked()
{
    gSettings->setValue("IsFullScreen", false);
}

void MainWindow::on_radioButton_theme_light_clicked()
{
    gSettings->setValue("ThemeDark", false);
}

void MainWindow::on_radioButton_theme_light_toggled(bool checked)
{
    if(!checked){//深色主题
        gSettings->setValue("ThemeDark", true);
    }else{//浅色主题
        /*默认深色主题,浅色切换如果需要支持多种主题使用qss样式表文件切换方便*/
    }
    QString theme = checked ? "light" : "dark";
    qDebug() << "#MainWindow:theme"<<theme;
    // 1. 设置根控件的动态属性
    this->setProperty("theme", theme);

    // 2. 递归设置所有子控件  QList 是一个基于指针的容器,遍历时可能会导致容器被分离
    QList<QWidget*> allWidgets = this->findChildren<QWidget*>();
    foreach (QWidget *widget, allWidgets) {
        widget->setProperty("theme", theme);
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
        widget->update();
    }
    // 3. 强制刷新样式（Qt6中这一步非常重要）
    this->ensurePolished();

    // 4. 递归刷新所有子控件
    for(QWidget *widget : std::as_const(allWidgets)) {
        widget->ensurePolished();
    }

    // 重要：更新样式
    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();
}

void MainWindow::on_radioButton_car_warning_off_clicked()
{
    gSettings->setValue("CarWarning", false);
}

void MainWindow::on_radioButton_car_warning_off_toggled(bool checked)
{
    if(!checked) gSettings->setValue("CarWarning", true);
    Session session(sModuleUser,"onCarWarningClicked",checked);
    gControl.SendAndWaitResult(session);
}

void MainWindow::on_radioButton_log_debug_clicked()
{
    SetLogLevel(LogLevel::Debug);
}

void MainWindow::on_radioButton_log_info_clicked()
{
    SetLogLevel(LogLevel::Info);
}

void MainWindow::on_radioButton_log_warning_clicked()
{
    SetLogLevel(LogLevel::Warning);
}

void MainWindow::on_radioButton_log_error_clicked()
{
    SetLogLevel(LogLevel::Error);
}

#include "ui/AboutDialog.h"
void MainWindow::on_pushButton_about_clicked()
{
    AboutDialog dialog(this);
    dialog.exec();
}

#pragma endregion

#pragma region PageProjectHub项目页

#include"dialog/CreateNewProject.h"
void MainWindow::on_action_create_project_triggered()
{
    CreateNewProject dialog(this);
    if(dialog.exec() == QDialog::Accepted){
        currentProjectItem = dialog.project;
        //添加json数据
        QJsonObject projects = gTaskManager.data[cKeyContent].toObject();
        projects[currentProjectItem.name] = currentProjectItem.ToJsonObject();
        gTaskManager.data[cKeyContent] = projects;
        this->AddProjectWidget(currentProjectItem);
        this->onEnterProjectClicked(currentProjectItem);
    }
}

void MainWindow::on_checkBox_project_sort_clicked(bool checked)
{
    qDebug() << "#MainWindow::on_checkBox_project_sort_clicked(bool checked)" <<checked;
    if (checked) {// 按时间排序
        std::sort(projectItemWidget.begin(),projectItemWidget.end(),[](ProjectItemCheckBox* a,ProjectItemCheckBox* b){
            return a->project.getTime() > b->project.getTime();
        });
    }
    else {
        std::sort(projectItemWidget.begin(),projectItemWidget.end(),[](ProjectItemCheckBox* a, ProjectItemCheckBox* b){
            return a->project.name.toLower() > b->project.name.toLower();
        });
    }
    // 存储所有widget的当前位置
    QHash<QWidget*, QPoint> oldPositions;
    for (int i = 0; i < projectItemWidget.size(); ++i) {
        oldPositions[projectItemWidget[i]] = projectItemWidget[i]->pos();
    }
    UpdateLayoutProjectWidget();
    // 为每个widget添加移动动画
    for (int i = 0; i < projectItemWidget.size(); ++i) {
        QWidget* widget = projectItemWidget[i];
        QPoint newPos = widget->pos();
        QPoint oldPos = oldPositions[widget];

        if (oldPos != newPos) {
            QPropertyAnimation* animation = new QPropertyAnimation(widget, "pos");
            animation->setStartValue(oldPos);
            animation->setEndValue(newPos);
            animation->setDuration(300);
            animation->setEasingCurve(QEasingCurve::OutCubic);
            animation->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
}


void MainWindow::on_dateTimeEdit_project_begin_dateTimeChanged(const QDateTime &dateTime)
{
    gSearchStartDateTime = dateTime;//获取开始时间,保证用户选择一次时间后启用时间筛选的功能
}

void MainWindow::on_pushButton_project_search_ok_clicked()
{
    QString nameFilter = ui.lineEdit_project_search_name->text().trimmed();

    //遍历所有项目进行过滤
    foreach(ProjectItemCheckBox* item , projectItemWidget) {
        bool show = true;
        if (!nameFilter.isEmpty()) {
            show = item->project.name.contains(nameFilter, Qt::CaseInsensitive);//英文部分不区分大小写
        }
        if (show && gSearchStartDateTime.isValid()) {
            show = item->project.getTime() >= gSearchStartDateTime && item->project.getTime() <= ui.dateTimeEdit_project_end->dateTime();
        }
        item->setVisible(show);
    }
}

void MainWindow::on_pushButton_project_delete_ok_clicked()
{
    foreach (auto button, projectItemWidget) {
        if (button->isChecked()) {
            //gTaskManager.DeleteProject(button->project.name);
            Session session(sModuleUser, "DeleteProject", button->project.ToJsonObject());
            if (gControl.SendAndWaitResult(session)) {
                projectItemWidget.removeOne(button);
            }else {
                qWarning() << "删除项目失败" << button->project.ToJsonObject()<< session.message;
                return;//失败
            }
        }
    }
    UpdateLayoutProjectWidget();
}


void MainWindow::on_pushButton_project_delete_cancel_clicked()
{
    foreach (auto button , projectItemWidget) {
        if (button->isChecked()) button->SetChecked(false);
    }
}
/*任务参数页设置*/
void MainWindow::on_comboBox_city_activated(int index)
{
    //0默认是自定义,清空选择
    if(index ==0){
        ui.comboBox_task_param_line_name->clear();
        ui.comboBox_task_param_between_name_before->clear();
        ui.comboBox_task_param_between_name_after->clear();

        ui.comboBox_task_param_line_name->setEditable(true);
        ui.comboBox_task_param_between_name_before->setEditable(true);
        ui.comboBox_task_param_between_name_after->setEditable(true);
        return;
    } else {
        ui.comboBox_task_param_line_name->setEditable(false);
        ui.comboBox_task_param_between_name_before->setEditable(false);
        ui.comboBox_task_param_between_name_after->setEditable(false);
    }
    QString city = ui.comboBox_city->currentText();
    if(city.isEmpty()) return;
    QJsonArray lines = citySubwayInfo.value(city).toArray();
    ui.comboBox_task_param_line_name->clear();
    for (const QJsonValue& value : lines) {
        QJsonObject line = value.toObject();
        ui.comboBox_task_param_line_name->addItem(line.value("ln").toString());
    }
    //默认第一个站
    //on_comboBox_task_param_line_name_activated(index - 1);
    QJsonObject line = lines[0].toObject();
    //QString line_name = line.value("ln").toString();
    QJsonArray stations = line.value("st").toArray();
    ui.comboBox_task_param_between_name_before->clear();
    ui.comboBox_task_param_between_name_after->clear();
    // 方法1：直接遍历
    for (const QJsonValue& value : stations) {
        QString station = value.toString();
        ui.comboBox_task_param_between_name_before->addItem(station);
        ui.comboBox_task_param_between_name_after->addItem(station);
    }
}


void MainWindow::on_comboBox_task_param_line_name_activated(int index)
{
    if(ui.comboBox_city->currentIndex() == 0) return;
    QString city = ui.comboBox_city->currentText();
    if (city.isEmpty()) return;
    QJsonArray lines = citySubwayInfo.value(city).toArray();
    QJsonObject line = lines[index].toObject();
    //QString line_name = line.value("ln").toString();
    QJsonArray stations = line.value("st").toArray();
    ui.comboBox_task_param_between_name_before->clear();
    ui.comboBox_task_param_between_name_after->clear();
    // 方法1：直接遍历
    for (const QJsonValue& value : stations) {
        QString station = value.toString();
        ui.comboBox_task_param_between_name_before->addItem(station);
        ui.comboBox_task_param_between_name_after->addItem(station);
    }
}

void MainWindow::on_pushButton_task_param_content_tunnel_diameter_sub_1_clicked()
{
    ui.doubleSpinBox_task_param_content_tunnel_diameter->stepBy(-10);
}

void MainWindow::on_pushButton_task_param_content_tunnel_diameter_add_1_clicked()
{
    ui.doubleSpinBox_task_param_content_tunnel_diameter->stepBy(10);
}

void MainWindow::on_pushButton_task_param_starting_ring_sub_50_clicked()
{
    ui.spinBox_task_param_starting_ring_number->stepBy(-50);
}


void MainWindow::on_pushButton_task_param_starting_ring_add_50_clicked()
{
    ui.spinBox_task_param_starting_ring_number->stepBy(50);
}


void MainWindow::on_pushButton_task_param_start_mileage_sub_50_clicked()
{
    ui.spinBox_task_param_start_mileage->stepBy(-50);
}


void MainWindow::on_pushButton_task_param_start_mileage_add_50_clicked()
{
    ui.spinBox_task_param_start_mileage->stepBy(50);
}


void MainWindow::on_pushButton_task_param_segment_width_sub_1_clicked()
{
    ui.doubleSpinBox_task_param_segment_width->stepBy(-10);
}


void MainWindow::on_pushButton_task_param_segment_width_add_1_clicked()
{
    ui.doubleSpinBox_task_param_segment_width->stepBy(10);
}

void MainWindow::on_pushButton_task_param_first_page_next_step_clicked() {
    QString name = ui.lineEdit_task_param_content_name->text().trimmed(); // 移除首尾空格
    // 检查文件名是否为空
    if(name.isEmpty()){
        ToolTip::ShowText(tr("任务名称不能为空"));
        return;
    }
    // 检查文件名合法性
    static QRegularExpression regex("[\\\\/:*?\"<>|]"); // Windows文件名非法字符
    if(name.contains(regex)){
        ToolTip::ShowText(tr("任务名称包含非法字符（\\ / : * ? \" < > |）"));
        return;
    }

    gTaskFileInfo->name = name;
    gTaskFileInfo->data[JSON_TASK_NAME] = name;
    QJsonObject content = gTaskFileInfo->data[JSON_TASK_CONTENT].toObject();
    content[JSON_DEVICE_TYPE] = __DEVICE_TYPE__;
    content[JSON_DIAMETER] = ui.doubleSpinBox_task_param_content_tunnel_diameter->value();
    
    QString line_name;
    if (ui.comboBox_city->currentIndex() == 0) {//不是自定义的名称
        line_name = ui.comboBox_city->currentText();//前缀加入城市
    }
    content[JSON_TMP_LINE_NAME] = line_name + ui.comboBox_task_param_line_name->currentText();
    content[JSON_TMP_BETWEEN_NAME] = ui.comboBox_task_param_between_name_before->currentText() + "-" + ui.comboBox_task_param_between_name_after->currentText();
    QString line_type;
    switch (ui.comboBox_task_param_line_type->currentIndex()) {
    case 0:line_type = "Left"; break;
    case 1:line_type = "Right"; break;
    case 2:line_type = "Up"; break;
    case 3:line_type = "Down";break;
    default:line_type = "Left";break;
    }
    content[JSON_TMP_LINE_TYPE] = line_type;

    content[JSON_START_RING] = ui.spinBox_task_param_starting_ring_number->value();
    content[JSON_START_MILEAGE] = ui.spinBox_task_param_start_mileage->value();
    content[JSON_SEGMENT_WIDE] = ui.doubleSpinBox_task_param_segment_width->value();

    ui.label_current_task_name->setText(tr("当前任务:%1").arg(name));
    ui.label_current_task_name->show();
    gTaskFileInfo->data[JSON_TASK_CONTENT] = content;
    ui.stackedWidget_task_param->setCurrentWidget(ui.page_task_param_last);
}

void MainWindow::on_pushButton_tsak_param_first_page_cancel_clicked()
{
    ui.StackedWidgetProjectHub->setCurrentWidget(ui.ProjectHome);//取消返回项目选择
}

void MainWindow::on_comboBox_parameter_templates_activated(int index)
{
    qDebug() << "MainWindow::on_comboBox_parameter_templates_activated(int "<<index;
    QJsonObject param = parameterTemplatesInfo.at(index).toObject();
    if(param.isEmpty())return;
    QJsonObject content = gTaskFileInfo->data[JSON_TASK_CONTENT].toObject();
    // content = content.unite(param);// 直接合并两个对象,如果有重复的 key 则使用 param 中的值

    QVariantMap map = param.toVariantMap();
    map.insert(content.toVariantMap());
    content = QJsonObject::fromVariantMap(map);

    gTaskFileInfo->data[JSON_TASK_CONTENT] = content;
}


void MainWindow::on_pushButton_task_param_last_page_previous_step_clicked() {
    ui.stackedWidget_task_param->setCurrentWidget(ui.page_task_param_first);
}


void MainWindow::on_pushButton_task_param_last_page_next_step_prestart_clicked() {
    QJsonObject content = gTaskFileInfo->data[JSON_TASK_CONTENT].toObject();

    content[JSON_TEMPLATE] = ui.comboBox_parameter_templates->currentText();
    content[JSON_ACCURACY] = ui.spinBox_scanner_accuracy->value();
    content[JSON_SPEED] = ui.spinBox_car_travel_speed->value();
    content[JSON_DIRECTION] = gControl.carDirection ? "Forward" : "Backward"; //0后退 1 前进

    if (gTrolley) {
        bool isRatedMileageOn = ui.radioButton_car_rated_mileage_on->isChecked();
        gTrolley->isRatedMileage = isRatedMileageOn;
        if (isRatedMileageOn) {
            int carRatedMileageValue = ui.spinBox_car_rated_mileage->value();
            content[JSON_CAR_RATED_MILEAGE] = carRatedMileageValue;
            gTrolley->carRatedMileage = carRatedMileageValue;
        }
    }

    gTaskFileInfo->data[JSON_TASK_CONTENT] = content;
    ui.StackedWidgetProjectHub->setCurrentWidget(ui.TaskAcquisition);
}


void MainWindow::on_pushButton_task_param_last_page_cancel_clicked() {
    ui.StackedWidgetProjectHub->setCurrentWidget(ui.ProjectHome);
}


void MainWindow::on_radioButton_car_obstacle_avoidance_enabled_clicked()
{
    ToolTip::ShowText(tr("设置雷达避障行为待支持"), 5000);

    //Session session(sModuleSerial, "SetCarObstacleAvoidance", true);
    //if (gControl.SendAndWaitResult(session)) {
    //} else {
    //    ToolTip::ShowText(tr("设置避障行为失败"), -1);
    //}
}


void MainWindow::on_radioButton_car_obstacle_avoidance_close_clicked()
{
    ToolTip::ShowText(tr("设置雷达避障行为待支持"), 5000);

    //Session session(sModuleSerial, "SetCarObstacleAvoidance", false);
    //if (gControl.SendAndWaitResult(session)) {
    //} else {
    //    ToolTip::ShowText(tr("设置避障行为失败"), -1);
    //}
}


void MainWindow::on_radioButton_realtime_parsing_enabled_clicked()
{
    Session session(sModuleUser, "SetRealtimeParsing", true);
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(tr("设置实时解析失败"), -1);
    }
}


void MainWindow::on_radioButton_realtime_parsing_off_clicked()
{
    Session session(sModuleUser, "SetRealtimeParsing", false);
    if (gControl.SendAndWaitResult(session)) {
    } else {
        ToolTip::ShowText(tr("设置实时解析失败"), -1);
    }
}

void MainWindow::on_radioButton_car_forward_clicked()
{
    //setCheckable(false) 没有的时候,应该禁用或者不显示
    if (gTrolley && gControl.carDirection == false) {//如果车辆方向不对,则先改变方向(不会影响车辆方向)
        QJsonObject obj;
        obj["code"] = 0x0D;//serial::CAR_CHANGING_OVER;
        QByteArray data;
        obj["data"] = data.toStdString().c_str();
        Session session(sModuleSerial, "SetParamsByCode", obj);
        if (gControl.SendAndWaitResult(session)) {
            gControl.carDirection = true;
        } else {
            ToolTip::ShowText(tr("设置车辆方向失败"), -1);
            ui.radioButton_car_forward->setChecked(false);
        }
    }
}


void MainWindow::on_radioButton_car_backward_clicked()
{
    if (gTrolley && gControl.carDirection == true) {//如果车辆方向不对,则先改变方向(不会影响车辆方向)
        QJsonObject obj;
        obj["code"] = 0x0D;//serial::CAR_CHANGING_OVER;
        QByteArray data;
        obj["data"] = data.toStdString().c_str();
        Session session(sModuleSerial, "SetParamsByCode", obj);
        if (gControl.SendAndWaitResult(session)) {
            gControl.carDirection = false;
        } else {
            ToolTip::ShowText(tr("设置车辆方向失败"), -1);
            ui.radioButton_car_backward->setChecked(false);
        }
    }
}


void MainWindow::on_radioButton_camera_format_jpg_clicked()
{
    SetCameraFormat("jpg");
}


void MainWindow::on_radioButton_camera_format_jpeg_clicked()
{
    SetCameraFormat("jpeg");
}


void MainWindow::on_radioButton_camera_format_png_clicked()
{
    SetCameraFormat("png");
}


void MainWindow::on_radioButton_camera_format_bmp_clicked()
{
    SetCameraFormat("bmp");
}


void MainWindow::on_radioButton_camera_format_raw_clicked()
{
    SetCameraFormat("raw");
}

void MainWindow::on_pushButton_test_clicked()
{
    ToolTip::ShowText("测试");
}

#pragma endregion

void MainWindow::_retranslate()
{
    qDebug()<<"void MainWindow::retranslate() 更新语言显示,主要是ui文件,和一些qss的文本";
    ui.retranslateUi(this);
}

void MainWindow::UpdateCitySubwayInfo(const QString& dirPathCity)
{
    QDir dir(dirPathCity);
    // 获取所有.json文件
    QStringList filters;
    filters << "*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    // 遍历每个json文件
    foreach(const QFileInfo& fileInfo,files) {
        QFile file(fileInfo.absoluteFilePath());
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << tr("无法读取文件:") << fileInfo.absoluteFilePath();
            continue;
        }
        // 读取文件内容
        QByteArray data = file.readAll();
        QString filename = fileInfo.baseName();
        file.close();
        // 解析JSON
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            qWarning() << tr("无法解析json数据") << data;
            continue;
        }
        QJsonObject obj = doc.object();
        QJsonArray lines_array = obj["l"].toArray();
        QJsonArray subway_lines;
        foreach(auto line_ref,lines_array) {
            QJsonObject line = line_ref.toObject();
            //qDebug() << line;
            QString line_name = line.value("ln").toString();
            if (line_name.contains(QChar::ReplacementCharacter)) {
                qWarning() << "⚠️ 非法线路名称（不是 UTF-8 编码）:" << line_name;
                continue;
            }
            QJsonArray st_array = line.value("st").toArray();
            QJsonArray st_names;
            foreach (auto st_ref , st_array) {
                QJsonObject st_obj = st_ref.toObject();
                QString name = st_obj.value("n").toString();
                st_names.append(name);
            }
            // qDebug() << "插入线路名称:" << line_name << "站点数量:" << st_names.size();
            QJsonObject line_obj;
            line_obj["ln"] = line_name;
            line_obj["st"] = st_names;
            subway_lines.append(line_obj);
        }
        //相同字符大小的线路名称,在Debug模式下,会出现报错 ASSERT: "lhs.size() == rhs.size()" 
        citySubwayInfo.insert(filename,subway_lines);
    }
    ui.comboBox_city->clear();
    ui.comboBox_city->addItem(tr("自定义"));
    ui.comboBox_city->addItems(citySubwayInfo.keys());
}

void MainWindow::UpdateCameraFormat(const QString &format)
{
    if(format == "jpg") ui.radioButton_camera_format_jpg->setChecked(true);
    else if(format == "jpeg") ui.radioButton_camera_format_jpeg->setChecked(true);
    else if(format == "png") ui.radioButton_camera_format_png->setChecked(true);
    else if(format == "bmp") ui.radioButton_camera_format_bmp->setChecked(true);
    else if(format == "raw") ui.radioButton_camera_format_raw->setChecked(true);
    else ui.radioButton_camera_format_jpg->setChecked(true);
}

void MainWindow::SetCameraFormat(const QString &format)
{
    Session session(sModuleCamera, "SetCameraFormat", format);
    if(gControl.SendAndWaitResult(session)){
    }else{
        ToolTip::ShowText(tr("设置相机照片格式%1失败").arg(format),-1);
    }
}

#include"button/ParamItemQRadioBox.h"
void MainWindow::UpdateLayoutParamTemplate()
{
    // 清除现有项目
    while (QLayoutItem* item = ui.LayoutParamTemplate->takeAt(0)){
        ui.LayoutParamTemplate->removeItem(item);
        item->widget()->deleteLater();
    }
    if(parameterTemplatesInfo.isEmpty()){
        QJsonObject info;
        info["name"] = "默认";
        parameterTemplatesInfo.append(info);
        parameterTemplatesInfo.append(info);
        parameterTemplatesInfo.append(info);
    }
    for(auto i =0; i< parameterTemplatesInfo.size();++i){
        QJsonObject param = parameterTemplatesInfo.at(i).toObject();
        ParamItemQRadioBox *button = new ParamItemQRadioBox(i,param,this->ui.WidgetParamTemplate);
        ui.LayoutParamTemplate->addWidget(button);
    }
    // 强制更新布局
    // ui.WidgetParamTemplate->adjustSize();
    // ui.LayoutParamTemplate->update();
}

void MainWindow::CurrentParamTemplateChanged(int id)
{
    qDebug()<< "CurrentParamTemplateChanged(int "<<id;
    QJsonObject param = parameterTemplatesInfo.at(id).toObject();
    qDebug() << param;
}
