#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <csignal>
#include <cstdlib>

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>

// 全局标志，用于标记是否正在退出
static bool g_isExiting = false;

// 测试对象 - 使用单例模式避免全局对象析构顺序问题
class TestObject {
public:
    static TestObject& instance() {
        static TestObject instance;
        return instance;
    }

    void cleanup() {
        if (!m_cleaned) {
            qDebug() << "TestObject 清理";
            OutputDebugStringA("TestObject cleaned\n");
            m_cleaned = true;
        }
    }

private:
    TestObject() {
        qDebug() << "TestObject 构造";
        OutputDebugStringA("TestObject constructed\n");
    }

    ~TestObject() {
        cleanup();
        qDebug() << "TestObject 析构";
        OutputDebugStringA("TestObject destructed\n");
    }

    TestObject(const TestObject&) = delete;
    TestObject& operator=(const TestObject&) = delete;

    bool m_cleaned = false;
};

// 日志类
class Log {
public:
    Log() {
        OutputDebugStringA("Log Constructor\n");
        qDebug() << "Log 构造";
    }

    ~Log() {
        OutputDebugStringA("Log Destructor\n");
        qDebug() << "Log 析构";
    }
};

// 控制台事件处理函数
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType) {
    QString msg;
    switch (dwCtrlType) {
    case CTRL_C_EVENT:
        msg = "Ctrl+C";
        break;
    case CTRL_BREAK_EVENT:
        msg = "Ctrl+Break";
        break;
    case CTRL_CLOSE_EVENT:
        msg = "控制台关闭";
        break;
    case CTRL_LOGOFF_EVENT:
        msg = "用户注销";
        break;
    case CTRL_SHUTDOWN_EVENT:
        msg = "系统关机";
        break;
    default:
        msg = "未知事件";
        break;
    }

    qDebug() << "收到控制台事件:" << msg;
    OutputDebugStringA(("Console event: " + msg + "\n").toUtf8().constData());

    if (!g_isExiting) {
        g_isExiting = true;
        qDebug() << "启动优雅退出...";

        // 执行清理
        TestObject::instance().cleanup();

        // 延迟退出，确保清理完成
        QTimer::singleShot(1000, []() {
            qDebug() << "退出应用程序";
            QCoreApplication::quit();
            });
    }

    // 返回 TRUE 表示我们处理了这个事件
    // 返回 FALSE 会让系统继续默认处理
    return TRUE;
}

// 优雅退出函数
void gracefulShutdown() {
    if (g_isExiting) return;

    g_isExiting = true;
    qDebug() << "=== 开始优雅关闭 ===";

    // 执行所有清理操作
    TestObject::instance().cleanup();

    qDebug() << "=== 优雅关闭完成 ===";
}

int main(int argc, char* argv[]) {
    // 设置控制台处理程序
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        qDebug() << "无法设置控制台处理程序";
    }

    QCoreApplication app(argc, argv);

    // 创建局部日志对象（会在main结束时析构）
    Log log;

    qDebug() << "程序启动！";
    qDebug() << "测试方法:";
    qDebug() << "1. 按 Enter 键正常退出";
    qDebug() << "2. 按 Ctrl+C 测试信号退出";
    qDebug() << "3. 直接关闭控制台窗口";

    // 注册退出清理函数
    std::atexit([]() {
        qDebug() << "atexit 函数被调用";
        gracefulShutdown();
        });

    // 设置信号处理
    std::signal(SIGINT, [](int sig) {
        qDebug() << "收到 SIGINT 信号";
        gracefulShutdown();
        QCoreApplication::quit();
        });

    std::signal(SIGTERM, [](int sig) {
        qDebug() << "收到 SIGTERM 信号";
        gracefulShutdown();
        QCoreApplication::quit();
        });

    // 测试正常退出 - 按回车键退出
    QObject::connect(&app, &QCoreApplication::aboutToQuit, []() {
        qDebug() << "QCoreApplication 即将退出";
        gracefulShutdown();
        });


    int result = app.exec();

    qDebug() << "应用程序退出，返回码:" << result;

    // 在main结束前再次确保清理
    gracefulShutdown();

    return result;
}