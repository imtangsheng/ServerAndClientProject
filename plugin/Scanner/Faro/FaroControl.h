#ifndef FAROCONTROL_H
#define FAROCONTROL_H

#include <QObject>
#include <QDateTime>
#include <QFileSystemWatcher>
#include "iFaro.h"

// 定义回调函数类型
using ScanCompletedCallback = std::function<void()>; // 简单回调，无参数
// 如果需要传递参数，可以定义为：std::function<void(VARIANT*, int)> 或其他类型

class FaroControl : public QObject
{
    Q_OBJECT
public:
    explicit FaroControl(QObject *parent = nullptr);
    ~FaroControl();

    QString ip;
    QString flsDir;
    bool isConnect();
    int Connect() const;
    int SetParameters(QJsonObject param);
    int ScanStart();//螺旋扫描下,启动但不记录数据
    int ScanRecord();//螺旋扫描下,开始记录数据
    int ScanPause();
    int ScanStop();
    int GetScanPercent();
    int shutdown();

    void SetScanCompletedCallback(const ScanCompletedCallback& callback);
    int errorNumbers = 0;

    QAtomicInteger<bool> isNextStopToggled = false;//The next stop triggers完成一个文件,判断是否停止的标志
    QFileSystemWatcher watcher;
    void startMonitoring(const QString& path) {
        if (!QDir().exists(path)) {
            qWarning() << "扫描文件监控文件夹路径不存在:" << path;
            return;
        }
        if (!watcher.directories().contains(path)) {
            qDebug() << "扫描文件监控文件夹路径:" << path;
            watcher.addPath(path);
        }
    }
    void stopMonitoring() {
        watcher.removePaths(watcher.directories());//移除所有目录
    }
public slots:
    void onDirectoryChanged(const QString& path);
signals:
};

#endif // FAROCONTROL_H
