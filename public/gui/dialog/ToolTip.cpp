#include "ToolTip.h"
#include "ui_ToolTip.h"

#ifdef _WIN32
// #include "public/utils/windows_utils.h"
#include <Windows.h>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")
void SetupAcrylicEffect(QWidget *win) {
    // 使用 Windows 10 Acrylic 效果
    HWND hwnd = (HWND)win->winId();
    struct ACCENTPOLICY {
        int nAccentState;
        int nFlags;
        int nColor;
        int nAnimationId;
    };
    struct WINCOMPATTRDATA {
        int nAttribute;
        PVOID pData;
        ULONG ulDataSize;
    };
    ACCENTPOLICY policy = {3, 2, 0x800000, 0}; // ACCENT_ENABLE_ACRYLICBLURBEHIND
    WINCOMPATTRDATA data = {19, &policy, sizeof(policy)}; // WCA_ACCENT_POLICY
    using SetWindowCompositionAttributePtr = BOOL(WINAPI *)(HWND, WINCOMPATTRDATA*);
    SetWindowCompositionAttributePtr setWindowCompositionAttribute =
        (SetWindowCompositionAttributePtr)GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetWindowCompositionAttribute");
    if (setWindowCompositionAttribute) {
        setWindowCompositionAttribute(hwnd, &data);
    }
}
#else
void SetupAcrylicEffect(QWidget *win) {
    win->setStyleSheet("background: rgba(0, 0, 0, 0.9);");
}

#endif

void SetBackground(QDialog* dialog,QWidget* background=parentBackground)
{
    // qDebug() << "&DialogSetCameraFocus::setBackground";
    // auto blurBackground = new AcrylicFrostedGlassWindow();
    auto blurBackground = new QWidget();
    // 确保窗口关闭时自动删除
    blurBackground->setAttribute(Qt::WA_DeleteOnClose);
    blurBackground->setWindowFlags(Qt::FramelessWindowHint);
    blurBackground->setAttribute(Qt::WA_TranslucentBackground);
    blurBackground->resize(background->size());
    // blurBackground->setStyleSheet("background: rgba(0, 0, 0, 0.9);");
    blurBackground->move(background->mapToGlobal(QPoint(0, 0)));
    SetupAcrylicEffect(blurBackground);
    blurBackground->raise();
    blurBackground->show();
    // 自动关闭和清理
    // QObject::connect(dialog, &QWidget::destroyed, blurBackground, &QWidget::deleteLater);
    QObject::connect(dialog, &QDialog::finished, blurBackground, &QWidget::deleteLater);//信号的参数可以比槽函数的参数多,但不能少 当信号的参数比槽函数多时,多余的参数会被忽略
}

ToolTip::ToolTip(QWidget *parent): ToolTip(Ok, "", "", -1, parent)
{
}

ToolTip::ToolTip(TipType type, const QString &title, const QString &message, int msecShowTime, QWidget *parent)
    : QDialog(parent),ui(new Ui::ToolTip),_type(type)
{
    ui->setupUi(this);
    // 在你的Dialog构造函数中添加:
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint); // 设置无边框
    setAttribute(Qt::WA_TranslucentBackground); // 设置背景透明,不然会有黑边
    // 局部变量是在栈上分配的，会自动管理生命周期 当使用WA_DeleteOnClose时，会导致对象被删除两次：一次是属性触发的删除，一次是栈自动清理
    // 移除该属性后，对象的生命周期完全由栈管理，避免了双重删除的问题
    // setAttribute(Qt::WA_DeleteOnClose);// 窗口关闭时会自动删除对象，相当于在close事件中调用deleteLater()
    //确认和取消按钮的显示
    switch (_type) {
    case Ok:
        ui->pushButton_reject->hide();
        break;
    default:
        break;
    }

    ui->label_title_text->setText(title);
    ui->label_message->setText(message);
    if(msecShowTime > 0){
        timer.setSingleShot(true);
        connect(&timer,&QTimer::timeout, this, &ToolTip::accept);
        timer.start(msecShowTime);
    }
    SetBackground(this);
}

ToolTip::~ToolTip()
{
    if(timer.isActive()){
        timer.stop();
    }
    delete ui;
}

void ToolTip::ShowText(const QString &text, int msecShowTime)
{
    ShowText(tr("提示"), text, msecShowTime);
}

void ToolTip::ShowText(const QString &title, const QString &message, int msecShowTime)
{
/*
 * 当对话框关闭时，Qt 会因为 WA_DeleteOnClose 属性尝试删除这个对象
 * 但是这个对象是栈上的局部变量，不是用 new 分配的堆内存
 * 当函数结束时，局部变量会自动析构，导致二次删除，引发崩溃
 */
    // ToolTip *tooltip = new ToolTip(Ok, title, message, msecShowTime);
    // tooltip->exec();
    ToolTip tooltip(title, message, msecShowTime);
    // tooltip.setAttribute(Qt::WA_DeleteOnClose, false); // 取消自动删除
    tooltip.exec();
}
