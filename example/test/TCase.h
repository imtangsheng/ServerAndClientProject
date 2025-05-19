#pragma once

/* CRTP 设计一个设备流程处理基类方法
* 奇异递归模板模式（CRTP, Curiously Recurring Template Pattern）
*/
template <typename T>
class DeviceProcess
{
public:
    bool init() {
        return static_cast<T *>(this)->InitImpl();
    }
    bool open() {
        return static_cast<T *>(this)->OpenImpl();
    }
    bool close() {
        return static_cast<T *>(this)->CloseImpl();
    }
    bool start() {
        return static_cast<T *>(this)->StartImpl();
    }
    bool stop() {
        return static_cast<T *>(this)->StopImpl();
    }

};

#include <memory>  // 为了使用 std::unique_ptr
/*
* IMPL模式（Implementation模式）是一种用于隐藏类实现细节的方法
* 
*/
class DeviceProcessV2 {
public:
    DeviceProcessV2();
    ~DeviceProcessV2();
    bool init();
    bool open();
    bool close();
    bool start();
    bool stop();
private:
    class Impl;
    //QScopedPointer<Impl> pImpl;//不支持 不完整类型（incomplete type,可改 QSharedPointer
    std::unique_ptr<Impl> pImpl;//需要定义析构方法                                                                                                               
};
#include <QDebug>
class TCase : public DeviceProcess<TCase>
{
public:
    bool InitImpl() {
        qDebug() << "TCase InitImpl";
        return true;
    }
    bool OpenImpl() {
        qDebug() << "TCase OpenImpl";
        return true;
    }
    bool CloseImpl() {
        qDebug() << "TCase CloseImpl";
        return true;
    }
    bool StartImpl() {
        qDebug() << "TCase StartImpl";
        return true;
    }
    bool StopImpl() {
        qDebug() << "TCase StopImpl";
        return true;
    }


};
