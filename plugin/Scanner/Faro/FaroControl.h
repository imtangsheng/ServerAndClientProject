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
    QString ip{ "172.17.9.20" };
    QString flsDir;
    Atomic<int> ret;//法如执行反馈结果错误码  ErrorNumbers :int 在头文件中定义错误信息枚举
    bool isConnect();
    int Connect();
    Result SetParameters(QJsonObject param);
    int ScanStart();//螺旋扫描下,启动但不记录数据
    int ScanRecord();//螺旋扫描下,开始记录数据
    int ScanPause();
    int ScanStop();
    int GetScanPercent();
    int shutdown();

    int errorNumbers = 0;

    //直接调用 使用回调函数在执行特定顺序的任务
    Result OnStarted(const CallbackResult& callback = nullptr);
    Result OnStopped(const CallbackResult& callback = nullptr);
    QSharedPointer<ScanCompletedCallback> callbackStartedPtr;
    QSharedPointer<ScanCompletedCallback> callbackStoppedPtr;

    QAtomicInteger<bool> isNextStopToggled = false;//The next stop triggers完成一个文件,判断是否停止的标志
    QFileSystemWatcher watcher;
    QFileSystemWatcher watcherPreview;
    void startMonitoring(const QString& path) {
        QDir flsDir(path);
        if (!flsDir.exists()) {
            qWarning() << "扫描文件监控文件夹路径不存在:" << path;
            return;
        }
        if (!watcher.directories().contains(path)) {
            qDebug() << "扫描文件监控文件夹路径:" << path;
            watcher.addPath(path);
        }
        if (gSettings->value("IsRealtimePreview").toBool()) {
            flsDir.cdUp();
            QString previewPath = flsDir.absoluteFilePath(tr("正射影像图"));
            if (QDir().mkpath(previewPath)) {
                qDebug() << "创建预览文件夹:" << previewPath;
                if (!watcherPreview.directories().contains(previewPath)) {
                    qDebug() << "扫描预览文件监控文件夹路径:" << previewPath;
                    watcherPreview.addPath(previewPath);
                }
            }
        }

    }
    void stopMonitoring() {
        watcher.removePaths(watcher.directories());//移除所有目录
        watcherPreview.removePaths(watcherPreview.directories());
    }
public slots:
    void onDirectoryChanged(const QString& path);
    void addPreviewFile(const QString& path);
signals:
    void onScanStateChanged(StateEvent::State state);
};

#endif // FAROCONTROL_H
