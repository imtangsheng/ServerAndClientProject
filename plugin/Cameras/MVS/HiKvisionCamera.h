#pragma once
#include "Includes/MvCameraControl.h"
#define MAX_DEVICE_NUM          9
struct MvCamera
{
    QString id;
    QString image_path;
    void* handle{ NULL };
  
    MV_CC_DEVICE_INFO* info;/// \~chinese 设备信息                  \~english Device info
    QString path;//图片存储路径
    void savaImage() {};
};
#include "iCameraBase.h"
class HiKvisionCamera : public ICameraBase
{
public:
    ~HiKvisionCamera();

    bool initialize() final;
    Result SetCameraConfig(const QJsonObject& config) final;
    Result scan() final; // 预先浏览相机设备
    Result open() final; //打开相机
    Result close() final; //关闭相机
    Result start() final; //开始采集
    Result stop() final; //停止采集
    Result triggerFire() final; //软触发一次

    QString DeviceName() const final {
        static QString name{ "HiKvisionCamera" };
        return name;
    }
    bool SetImageFormat(const QString& format) final;
    //SDK内部调用
    const static int kDefaultCameraCount = 3;
    //相机内部的变量的声明
    unsigned int nRet = MV_OK;//统一的相机返回状态处理
    int camera_scan_count;//用于写入相机扫描个数的加载
    qint8 active_index{ 0 };//当前选中默认
    bool trigger_mode_enabled{ true }; //触发模式 软触发或者硬触发才会调用回调函数,连续出图是视频流一样调用回调函数

    //MvCamera* camera_info_array[MAX_DEVICE_NUM];// ch:MvCamera封装了常用接口 | en:MvCamera packed normal used interface
    QList<MvCamera*> camera_list;
    Result SetCameraParam(void* handle, const QJsonObject& param);
    Result SetTriggerMode(void* handle);
};
