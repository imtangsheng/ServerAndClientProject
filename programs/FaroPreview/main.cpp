#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFileInfo>
#include <QThread>
#include "RealtimeSolving.h"
#include "logger.h"


int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    
    // 安装消息处理钩子，重定向QDebug输出
#ifdef QT_NO_DEBUG
    Logger::getInstance()->init("../logs", "RealtimeSolving", Warning, true);
    Logger::getInstance()->installMessageHandler();
#endif // QT_DEBUG
    QCoreApplication::setApplicationName("RealtimeSolving");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("法如实时解算工具");
    parser.addHelpOption();
    parser.addVersionOption();

    // 添加命令行选项

    QCommandLineOption faroFileOption(QStringList() << "f" << "faroFile",
        "法如文件的绝对路径",
        "文件路径");
    QCommandLineOption taskDirOption(QStringList() << "t" << "taskDir",
        "任务文件夹路径",
        "目录路径");
    QCommandLineOption imagePathOption(QStringList() << "i" << "imagePath",
        "输出的图片文件夹路径",
        "目录路径");
    parser.addOption(faroFileOption);
    parser.addOption(taskDirOption);
    parser.addOption(imagePathOption);
    parser.process(app);

    // 获取参数值
    QString imagePath = parser.value(imagePathOption);
    QString faroFile = parser.value(faroFileOption);
    QString taskDir = parser.value(taskDirOption);
    // 检查必要参数是否提供
    if (faroFile.isEmpty()) {
        qWarning() << "错误:\n没有提供需要解析的法如文件路径,是否缺少参数'--faroFile' (or '-f').\n";
        QTextStream(stdout) << parser.helpText().toUtf8().data() << Qt::endl;//控制台显示
        QThread::sleep(5); // 暂停秒
        //parser.showHelp(); //显示帮助信息后会立即退出程序
        //system("pause"); // Windows系统下使用pause命令
        exit(1);//直接退出程序,app.exit没有退出,而是发出退出信号
        Q_UNREACHABLE();//用于标记一个不应该被执行到的代码路径

    }
    
    // 检查Faro文件是否存在
    QFileInfo faroFileInfo(faroFile);
    if (!faroFileInfo.exists()) {
        qWarning() << "错误:\n输入的法如文件路径不存在,请检查输入正确的文件路径:" << faroFile << Qt::endl;
        QThread::sleep(5); // 暂停3秒
        return 1;
        Q_UNREACHABLE();//用于标记一个不应该被执行到的代码路径
    }

    // 设置默认值
    if (taskDir.isEmpty()) {
        QDir dir = faroFileInfo.absoluteDir();
        dir.cdUp();
        taskDir = dir.absolutePath()+ ("/Task");
        //+ "/Task";// 将taskDir设置为faroFile目录的上一级目录
    }
    if (imagePath.isEmpty()) imagePath = QDir(faroFileInfo.absolutePath() + ("/../PointCloudImage")).absolutePath();
    
    // 图片服务器,可以浏览图片
     //ImageServer server;
     //if (!server.startServer("C:\\Users\\Tang\\Pictures\\", 12345)) {
     //    qFatal("ImageServer Failed to start server!");// 输出致命错误信息并终止应用程序 输出的错误信息作为参数传递给 qFatal() 宏，而不需要使用输出流操作符 <<。
     //}
    // 实时解算功能,生成图片 RealtimeSolving
    TaskFaroPart task;
    task.task_dir = taskDir;
    task.faro_file = faroFile;
    task.mileage_file = QDir(taskDir).filePath("mileage.txt");
    task.clinometer_file = QDir(taskDir).filePath("Inclinometer.txt");

    RealtimeSolving solving;
    qDebug() << "#法如文件实时解算生成图像,深度图和灰度图结果:";
    qDebug() << solving.writeFaroImage(task, imagePath);
    QThread::sleep(5); // 暂停
    return 0;  // 直接返回，不调用app.exec()
}
