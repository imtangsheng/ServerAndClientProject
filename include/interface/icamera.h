// ICameraBase.h
#pragma once

//声明图片格式
inline static QString g_image_format("jpg");

class ICameraBase {
public:
    ICameraBase() = default;
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

    QStringList camera_id_list;//保存的相机名称,用于打开相机
    QString has_image_format{"jpg,bmp"};//支持的图片格式

    virtual QString DeviceName() const {
        static QString name{"camera"};
        return name;
    }
    virtual bool SetImageFormat(const QString& format) {
        g_image_format = format;
        return true;
    }
protected:
    //用于支持tr的翻译
    static QString tr(const char* sourceText) {
        return QObject::tr(sourceText);
    }
};