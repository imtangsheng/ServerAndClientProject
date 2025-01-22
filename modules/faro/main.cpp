// main.cpp (服务器)
#include <QCoreApplication>
#include "imageserver.h"
#include "FaroHandle.h"

#include <qdatetime.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Set up code that uses the Qt event loop here.
    // Call a.quit() or a.exit() to quit the application.
    // A not very useful example would be including
    // #include <QTimer>
    // near the top of the file and calling
    // QTimer::singleShot(5000, &a, &QCoreApplication::quit);
    // which quits the application after 5 seconds.

    // If you do not need a running Qt event loop, remove the call
    // to a.exec() or use the Non-Qt Plain C++ Application template.

    // ImageServer server;
    // if (!server.startServer("C:\\Users\\Tang\\Pictures\\", 12345)) {
    //     qDebug() << "Failed to start server!";
    //     return -1;
    // }

    FaroHandle handle;
    std::vector<std::vector<PointCloudXYZCT>> pointcloudRes;

    QDateTime startTime = QDateTime::currentDateTime();
    qDebug() << "startTime: " << startTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
    handle.init();
    qDebug() << "init end" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
    handle.read_pointcloud_from_file("E:\\Scan001.fls",pointcloudRes);

    //unsigned __int64 start, end;
    //handle.getFaroFlsStartAndEndTime("E:\\Scan001.fls", start, end);
    //qDebug() << "StartAndEndTime: " <<start<<end<< QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    qDebug() << "endTime: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    qDebug() << "time: " << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
    qDebug() << "pointcloudRes size: " << pointcloudRes.size();

    //RealtimeSolving

    return a.exec();
}
