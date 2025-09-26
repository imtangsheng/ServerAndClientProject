#pragma once

#include <QThread>
#include <QMutex>

class FaroHandle : public QObject
{
    Q_OBJECT
public:
    FaroHandle(QObject* parent = nullptr);
    ~FaroHandle();
    QThread* thread;

    Session session;//记录当前最后的会话用于返回
    void Test();
/*槽函数在线程中运行,使用信号调用*/
public slots:
    void awake();
    void CreateCameraFocalByScanFile(const QJsonObject& in);

protected:
    int errorNumbers = 0;
   
    Result LoadFlsFile(QString filepath);
signals:
// 只有类自己可以发射(emit)该信号 但其他类仍然可以连接(connect)这个信号
// void finished(QPrivateSignal);//QPrivateSignal参数 - 这表明这是一个私有信号，只能由类本身发射
};