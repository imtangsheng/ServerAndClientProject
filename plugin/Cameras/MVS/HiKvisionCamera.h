#pragma once
#include "iCameraBase.h"
class HiKvisionCamera : public ICameraBase
{
public:
    HiKvisionCamera();
    ~HiKvisionCamera();

    bool initialize() final;
    Result SetCameraConfig(const QJsonObject& config) final;
    Result scan() final; // 预先浏览相机设备
    Result open() final; //打开相机
    Result close() final; //关闭相机
    Result start() final; //开始采集
    Result stop() final; //停止采集
    Result triggerFire() final; //软触发一次
};
