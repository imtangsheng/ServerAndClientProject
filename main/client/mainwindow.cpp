#include <QMouseEvent>
#include <QEvent>

#include <QActionGroup>

#include "mainwindow.h"

#include "ui/WebSocketWidget.h"
#include "ui/CameraWidget.h"
#include "ui/ScannerWidget.h"
#include "dialog/AcquisitionWizard.h"

#include "controller/user_controller.h"
QPointer<UserController> gUserController;
QPointer<WebSocketWidget> gWebSocket;
QPointer<CameraWidget>gCamera;

// 在源文件中定义全局变量
// Q_GLOBAL_STATIC_WITH_ARGS(QString, zh_CN, ("zh_CN"))
static const char* zh_CN = "zh_CN";
static const char* en_US = "en_US";

static bool lastDarkMode; // 记录上一次的主题状态

/**
 * Reads the content of the given style sheet file
 */
static QString ReadStyleSheet(const QString& FileName)
{
    QString Result;
    QFile StyleSheetFile(FileName);
    StyleSheetFile.open(QIODevice::ReadOnly);
    QTextStream StyleSheetStream(&StyleSheetFile);
    Result = StyleSheetStream.readAll();
    StyleSheetFile.close();
    return Result;
}

/**
 * 定义IWidget的嵌入界面的实现
 */
#include "ui/IWidget.h"
IWidget::IWidget(MainWindow *parent)
:mainWindow(parent)
{
    qDebug() <<"IWidget::IWidget(MainWindow *parent)";
    connect(mainWindow,&MainWindow::languageChanged,this,&IWidget::retranslate_ui);
}

void IWidget::initialize()
{
    qDebug() <<"IWidget::initialize()";
    if(buttonDeviceManager_){
        int index = mainWindow->ui->stackedWidget_DeviceManager_items_screen->addWidget(widgetDeviceManager_);
        mainWindow->ui->verticalLayout_DeviceManager_items_name->insertWidget(index,buttonDeviceManager_);
        connect(buttonDeviceManager_,&QPushButton::clicked,mainWindow,[&]{
            mainWindow->ui->stackedWidget_DeviceManager_items_screen->setCurrentWidget(widgetDeviceManager_);
        });
        mainWindow->ui->verticalLayout_AcquisitionGo_Monitor->addWidget(widgetAcquisitionMonitor_);

    }
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow 构造函数";
    ui->setupUi(this);
    // 设置窗口主题
    HWND hwnd = reinterpret_cast<HWND>(this->winId());
    lastDarkMode = IsSystemDarkMode() ? TRUE : FALSE;
    SetDarkMode(hwnd,lastDarkMode);

    ui->dockWidget_Settings->hide();
    //初始化加载相机模块
    gCamera = new CameraWidget(this);

    gWebSocket = new WebSocketWidget();
    int index = ui->stackedWidget_DeviceManager_items_screen->addWidget(gWebSocket->ui->widget_DeviceManager);
    ui->verticalLayout_DeviceManager_items_name->insertWidget(index,gWebSocket->ui->pushButton_tab_websokcet);
    connect(gWebSocket->ui->pushButton_tab_websokcet,&QPushButton::clicked, this, [&]{
        ui->stackedWidget_DeviceManager_items_screen->setCurrentWidget(gWebSocket->ui->widget_DeviceManager);
    });


    ui->verticalLayout_AcquisitionGo_Monitor->addWidget(gCamera->ui->AcquisitionMonitor);

    gScanner = new ScannerWidget(this);

    // ui->verticalLayout_AcquisitionGo_Monitor->addWidget(gScanner);
    gUserController = new UserController(this);
    connect(&gSouth,&south::ShareLib::signal_set_window_title,this,&MainWindow::setWindowTitle);
    connect(&gLog,&Logger::new_message,this,&MainWindow::show_message);


    ui->menuBar->setAttribute(Qt::WA_TranslucentBackground);
    ui->menuSettings->setAttribute(Qt::WA_TranslucentBackground);
    // ui->menuSettings->setWindowFlags(ui->menuSettings->windowFlags() | Qt::NoDropShadowWindowHint);
    ui->menuSettings->setWindowOpacity(0.2);
ui->menuBar->setWindowOpacity(0.2);
    // 创建互斥的复选框组
    QActionGroup* modeGroup = new QActionGroup(this);
    modeGroup->addAction(ui->action_LanguageChinese);
    modeGroup->addAction(ui->action_languageEnglish);

    show_message("这是一条调试信息",LogLevel::Info);
    show_message("这是一条警告信息",LogLevel::Warning);
    show_message("这是一条错误信息",LogLevel::Error);
    // QMenu *menu = new QMenu(this);
    // menu->addAction("file");
    // ui->pushButton_test->setMenu(menu);
    // 启用自定义标题栏
    // setProperty("_q_custom_title_bar_color", QColor(33, 150, 243)); // 蓝色
    // setWindowFlags(Qt::FramelessWindowHint);//无边框和标题栏
    // setWindowFlags(Qt::CustomizeWindowHint);//保留边框,但是顶部有白边
    // 设置窗口属性以支持边框调整
    // setAttribute(Qt::WA_Hover, true);//启用悬停事件，以便在鼠标移动时检测边框区域。
    // 安装事件过滤器以监听焦点变化
    ui->widget_title->installEventFilter(this);
    ui->widget_AcquisitionGo->installEventFilter(this);
    ui->widget_DeviceManager->installEventFilter(this);
    // installEventFilter(this);

    connect(this,&MainWindow::languageChanged,this,[&]{
        ui->retranslateUi(this);
    });


    auto StyleSheet = ReadStyleSheet("mainwindow.css");
    qDebug() << StyleSheet;
    // setStyleSheet(StyleSheet);
}

MainWindow::~MainWindow()
{
    delete gWebSocket;
    gWebSocket = nullptr;
    delete gCamera;
    delete gUserController;
    delete ui;
    qDebug() << "MainWindow 析构函数";
}


void MainWindow::show_message(const QString &message, LogLevel level)
{

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString levelStr;
    QColor color;
    switch (level) {
    case LogLevel::Error:
        levelStr = tr("[Error]");
        color = Qt::red;
        break;
    case LogLevel::Warning:
        levelStr = tr("[Warning]");
        color = Qt::yellow;
        break;
    default:
        color = QColor(128, 128, 128);// 灰色
        break;
    }
    qDebug() << int(level) <<message<<color.name();
    // ui->plainTextEdit_log->appendPlainText(QString("[%1] %2 %3").arg(timestamp).arg(levelStr).arg(message));
    ui->plainTextEdit_log->appendHtml(QString("<span style='color: %1'>[%2] %3 %4</span>").arg(color.name(),timestamp,levelStr,message));

}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);

    MSG *msg = static_cast<MSG*>(message);
    if (msg->message == WM_SETTINGCHANGE) {
        // 系统设置发生变化，可能是主题切换
        // static bool lastDarkMode; // 记录上一次的主题状态
        bool currentDarkMode = IsSystemDarkMode();
        if (currentDarkMode != lastDarkMode) {
            // 仅当主题状态发生变化时才更新
            lastDarkMode = currentDarkMode;
            SetDarkMode(reinterpret_cast<HWND>(this->winId()),IsSystemDarkMode());
            qDebug() << "System theme changed to" << (IsSystemDarkMode() ? "dark" : "light") << "mode";
        }
        return true; // 表示已处理该消息
    }

    return QMainWindow::nativeEvent(eventType, message, result);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // qDebug() << "MainWindow::eventFilter"<<event->type()<<obj->objectName();
    if(event->type() == QEvent::MouseButtonPress){
        if(obj == ui->widget_title){
            // qDebug() << "MainWindow::eventFilter ui->widget_title";
            ui->stackedWidget_MainWidget->setCurrentIndex(0);
        }else if(obj == ui->widget_AcquisitionGo){
            // qDebug() << "MainWindow::eventFilter ui->widget_AcquisitionGo";
            handle_acquisition_start();
        }else if(obj == ui->widget_DeviceManager){
            ui->stackedWidget_MainWidget->setCurrentWidget(ui->page_manger);
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::handle_acquisition_start()
{
    AcquisitionWizard* acquisition = new  AcquisitionWizard(this);
    // 连接信号
    connect(acquisition, &QDialog::accepted, this, [&]{
        qDebug()<<"AcquisitionWizard QWizard::accepted:";
        ui->stackedWidget_MainWidget->setCurrentWidget(ui->page_AcquisitionGo);
    });
    //取消信号
    connect(acquisition, &QDialog::rejected, this, [&]{
        qDebug()<<"AcquisitionWizard QWizard::rejected:";
    });

    acquisition->show();
    // int index2 = ui->stackedWidget_AcquisitionGo_SettingsScreens->addWidget(acquisition);
    // qDebug()<<"AcquisitionWizard:"<<index2;
    // ui->stackedWidget_AcquisitionGo_SettingsScreens->setCurrentIndex(index2);
}

void MainWindow::on_pushButton_Camera_clicked()
{
    gCamera->show();
}

void MainWindow::on_pushButton_language_switch_clicked()
{
    // const QString zh_CN = "zh_CN";
    // const QString en_US = "en_US";

    if(gSouth.language == zh_CN)
    {
        //切换英文
        qApp->removeTranslator(&g_translator);
        // ui->pushButton_language_switch->setText("中文");
        gSouth.language = en_US;
        qDebug()<<"当前显示语言Languages："<<gSouth.language;
    }else{
        if(g_translator.load(QString(":/i18n/%1").arg(zh_CN))){
            // ui->pushButton_language_switch->setText("English");
            gSouth.language = zh_CN;
            qApp->installTranslator(&g_translator);
            qDebug()<<"当前显示语言Languages："<<gSouth.language;
        }else{
            show_message(tr("fail language switch:%1").arg(zh_CN), LogLevel::Warning);
        }
    }
    emit languageChanged();
}


void MainWindow::on_actionEnglish_triggered()
{
    //切换英文
    qApp->removeTranslator(&g_translator);
    // ui->pushButton_language_switch->setText("中文");
    gSouth.language = en_US;
    qDebug()<<"当前显示语言Languages："<<gSouth.language;
    emit languageChanged();
}


void MainWindow::on_actionChinese_triggered()
{
    if(g_translator.load(QString(":/i18n/%1").arg(zh_CN))){
        // ui->pushButton_language_switch->setText("English");
        gSouth.language = zh_CN;
        qApp->installTranslator(&g_translator);
        qDebug()<<"当前显示语言Languages："<<gSouth.language;
    }else{
        show_message(tr("fail language switch:%1").arg(zh_CN), LogLevel::Warning);
    }

    emit languageChanged();
}


void MainWindow::on_pushButton_test_clicked()
{
    ui->dockWidget_Settings->show();
    // ui->dockWidget_Settings->setFloating(true);//Qt::CustomizeWindowHint
    // ui->dockWidget_Settings->setWindowFlags(Qt::FramelessWindowHint);//Qt::FramelessWindowHint
    ui->dockWidget_Settings->setAttribute(Qt::WA_TranslucentBackground);
    ui->dockWidget_Settings->setWindowOpacity(0.5);
    ui->dockWidget_Settings->move(ui->stackedWidget_MainWidget->mapToGlobal(QPoint(0,0)));
    ui->dockWidget_Settings->adjustSize();
}


void MainWindow::on_pushButton_AcquisitionBegins_clicked()
{
    gUserController->acquisition_begins();
}

