#pragma once

#include "icamera.h"
#include "DVPCamera.h"
#include <QThread>
//#pragma comment(lib, DVP_LIB_PATH)//"x64/DVPCamera64.lib") //该文件放到生成目录下 或启动cmake的链接
// 相机信息结构体
struct struCameraInfo {
    QString name;
    QString path;
    QString id;
    dvpCameraInfo info{};
    dvpHandle    handle{};//相机句柄
    dvpStreamState state{};//视频流状态

    //QMutex mutex;
    struCameraInfo() = default;
    void update(QJsonObject json) {
        if (json.contains("name")) name = json["name"].toString();
        if (json.contains("path")) path = json["path"].toString();
        if (json.contains("id")) id = json["id"].toString();
    }
    QJsonObject ToJson() const {
        QJsonObject json;
        if (!name.isEmpty()) json["name"] = name;
        if (!path.isEmpty()) json["path"] = path;
        if (!id.isEmpty()) json["id"] = id;
        return json;
    }
    /**
     * @brief 相等比较运算符
     * @param other 要比较的其他相机信息对象 用于支持QSet
     * @return 如果两个相机信息对象相等，返回true；否则返回false
     */
    bool operator==(const struCameraInfo& other) const {
        // 基于唯一ID比较是否相等
        return name == other.name;
    }
    /**
    * 采集图片保存
    */
    INT AcquiringImage(dvpFrame* pFrame, void* pBuffer) const;
};

// Add this after your CameraInfo class definition 用于支持QSet
//inline static size_t qHash(const struCameraInfo& info, uint seed = 0) {
//    return qHash(info.name, seed);
//}

class DvpLineScanCamera : public ICameraBase
{
public:
    //基类方法
    bool initialize() final;
    ~DvpLineScanCamera();
    Result SetCameraConfig(const QJsonObject& config) final;
    Result scan() final; // 预先浏览相机设备
    Result open() final; //打开相机
    Result close() final; //关闭相机
    Result start() final; //开始采集
    Result stop() final; //停止采集
    Result triggerFire() final; //软触发一次

    //任务类的方法,需要回应
    void start(const Session& session) final;
    void stop(const Session& session) final;

    //直接调用 使用回调函数在执行特定顺序的任务
    Result OnStarted(CallbackResult callback = nullptr) final;
    Result OnStopped(CallbackResult callback = nullptr) final;

    QJsonObject GetDeviceIdList() const final;
    QString DeviceName() const final {
        static QString name{ "DvpLineScanCamera" };
        return name;
    }
    //SDK内部调用
    const static int kDefaultCameraCount = 8;
    //相机内部的变量的声明
    dvpStatus resultDvp;//统一的相机返回状态处理
    dvpUint32 camera_scan_count;//用于写入相机扫描个数的加载
    qint8 active_index{ 0 };//当前选中默认
    //QStringList camera_id_list;//保存的相机名称,用于打开相机
    bool trigger_mode_enabled{ true }; //触发模式 软触发或者硬触发才会调用回调函数,连续出图是视频流一样调用回调函数

    struCameraInfo camera_info_array[kDefaultCameraCount]; //std::vector<int> cameraInfoArray(8);//也可以QSet<struCameraInfo> set_cameras;
    //设置所有相机要用的参数 commom task 的参数
    //通过句柄设置单个相机单个参数
    Result SetCameraParam(const dvpHandle& handle, const QJsonObject& param);
    // 线扫模式使能配置,需要关闭触发使能,打开线扫模式使能 在句柄有效后使用
    Result SetLineScanMode(const dvpHandle& handle);
    //相机内部调用
    Result Property();//仅在windows有效

};
