// ICameraBase.h
#pragma once

//声明图片格式
inline static StringChar s_image_format("jpg");

class ICameraBase {
public:
    virtual ~ICameraBase() = default;
    // 定义共享的接口方法
    virtual bool initialize() = 0;
    //设置所有相机要用的参数 commom task 的参数
    virtual Result SetCameraConfig(const QJsonObject& config) =0;
    //通过句柄设置单个相机单个参数
    virtual Result scan() = 0; // 预先浏览相机设备
    virtual Result open() = 0; //打开相机
    virtual Result close() = 0; //关闭相机
    virtual Result start() = 0; //开始采集
    virtual Result stop() = 0; //停止采集
    virtual Result triggerFire() = 0; //软触发一次

    QStringList devicesIdList;//保存的相机名称,用于打开相机
    QString &getImageFormat(){
        static QString format = "JPEG";
        return format;
    }
};