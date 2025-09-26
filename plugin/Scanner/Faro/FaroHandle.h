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

    Session session;//��¼��ǰ���ĻỰ���ڷ���
    void Test();
/*�ۺ������߳�������,ʹ���źŵ���*/
public slots:
    void awake();
    void CreateCameraFocalByScanFile(const QJsonObject& in);

protected:
    int errorNumbers = 0;
   
    Result LoadFlsFile(QString filepath);
signals:
// ֻ�����Լ����Է���(emit)���ź� ����������Ȼ��������(connect)����ź�
// void finished(QPrivateSignal);//QPrivateSignal���� - ���������һ��˽���źţ�ֻ�����౾����
};