#include "dvp_line_scan_camera.h"
#include <QFile>
#include <QDateTime>

//是否有效句柄
inline bool IsValidHandle(dvpHandle handle) {
    bool bValidHandle;
    dvpIsValid(handle, &bValidHandle);
    return bValidHandle;
}

// 全局回调函数
static INT AcquiringImageCallback(dvpHandle handle, dvpStreamEvent event, void* pContext, dvpFrame* pFrame, void* pBuffer) {
    // 记录开始时间
    QDateTime startTime = QDateTime::currentDateTime();
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << QThread::currentThreadId() << "采集回调函数调用 handle event" << handle << event;
    // 通过pContext访问设备对象
    struCameraInfo* device = static_cast<struCameraInfo*>(pContext);
    device->AcquiringImage(pFrame, pBuffer);
    // 计算执行时间(毫秒)
    int elapsed = startTime.msecsTo(QDateTime::currentDateTime());
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "回调函数执行时间:" << elapsed << "毫秒";
    return 0;
}
#include<QImage>
#include<QBuffer>
INT struCameraInfo::AcquiringImage(dvpFrame* pFrame, void* pBuffer) const {
    QString image_path = QString(path + "/%1#%2.%3").arg(pFrame->uTriggerId).arg(pFrame->uTimestamp).arg(g_image_format);
    qDebug() << QThread::currentThread() << "AcquiringImage"
        << "uFrameID" << pFrame->uFrameID << "userValue" << pFrame->userValue
        << "uTriggerId" << pFrame->uTriggerId << "uTimestamp" << pFrame->uTimestamp << "图片路径" << image_path;
    dvpSavePicture(pFrame, pBuffer, image_path.toLocal8Bit().data(), 100);//546毫秒 无压缩 比image约快4倍 bmp最快,151毫秒
    // 1. 将原始图像转为QImage
    const uchar* data = static_cast<const uchar*>(pBuffer);
    const int width = pFrame->iWidth;
    const int height = pFrame->iHeight;
    qDebug() << "width" << width << "height" << height << "format" << pFrame->format;
    QImage image;
    switch (pFrame->format) {
        // 直接对应格式
    case FORMAT_MONO:
        image = QImage(data, width, height, QImage::Format_Mono);
        break;
    case FORMAT_RGB24:
        image = QImage(data, width, height, QImage::Format_RGB888);
        break;
        // 需要通道交换的格式
    case FORMAT_BGR24:
        image = QImage(data, width, height, QImage::Format_BGR888);
        image = image.rgbSwapped();//返回一个新的 QImage
        break;
    default:
        qWarning() << "Unsupported format:" << pFrame->format;
        image = QImage();
        break;
    }
    // 2. 压缩为图片并转为Base64
    qDebug() << "image.size" << image.size() << image.sizeInBytes();


    //image.save(image_path);//2140毫秒,有压缩数据大约10倍
    // 2. 极限缩放（缩小到1%面积，约10%长宽）
    const double SCALE_FACTOR = 0.05; // 5%线性缩放 
    QSize minSize = image.size() * SCALE_FACTOR;
    minSize = minSize.expandedTo(QSize(32, 32)); // 保证最小32x32
    //缩略图
    QImage thumbnail = image.scaled(minSize,
        Qt::IgnoreAspectRatio,
        Qt::FastTransformation); // 速度优先
    qDebug() << "thumbnail.size" << thumbnail.size() << thumbnail.sizeInBytes();
    //tips:此处开始下一帧触发处理
    // 4. 动态压缩至目标大小（<50KB）
    QByteArray compressedData;//压缩数据
    QBuffer buffer(&compressedData);
    // 查看所有支持的写入格式
    //qDebug() << "Supported write formats:" << QImageWriter::supportedImageFormats();
    //Supported write formats : QList("bmp", "cur", "ico", "jfif", "jpeg", "jpg", "pbm", "pgm", "png", "ppm", "xbm", "xpm")
    // 查看所有支持的读取格式
    //qDebug() << "Supported read formats:" << QImageReader::supportedImageFormats();
    //Supported read formats : QList("bmp", "cur", "gif", "ico", "jfif", "jpeg", "jpg", "pbm", "pgm", "png", "ppm", "svg", "svgz", "xbm", "xpm")
    int quality = 40; // 初始低质量
    //thumbnail.save(&buffer, format.toLocal8Bit().data(), quality);
    thumbnail.save(&buffer, "JPG", quality);//自定义格式无压缩,时间慢,改为jpg最大压缩,最快151毫秒bmp,151M, jpg是26M 547毫秒
    //do {
    //    compressedData.clear();
    //    thumbnail.save(&buffer, format.toLocal8Bit().data(), quality); 
    //    quality -= 5;
    qDebug() << "compressedData.size" << compressedData.size() << "quality" << quality;
    //} while (compressedData.size() > 50 * 1024 && quality > 10); // 目标50KB

    // 5. 如果仍过大，转用二值化
    //if (compressedData.size() > 50 * 1024) {
    //    thumbnail = thumbnail.convertToFormat(QImage::Format_Mono);
    //    thumbnail.save(&buffer, "PNG"); // 二值PNG更小
    //}

    //Base64会将每3字节的二进制数据转换为4字节的ASCII字符，因此：理论膨胀率：4 / 3 ≈ 1.333（即增加约33.3 % ）
    QByteArray& byteArray = compressedData;
    //QBuffer buffer(&byteArray);
    //image.save(&buffer, format.toLocal8Bit().data(), 100); //QString->char*  质量压缩系数
    //QString base64Data = QString::fromLatin1(byteArray.toBase64().data());
    //qDebug() << "base64Data.size" << base64Data.size();
    //QJsonObject imageInfo;
    //imageInfo["id"] = info.UserID;
    //imageInfo["frameId"] = qint64(pFrame->uFrameID);
    //imageInfo["timestamp"] = qint64(pFrame->uTimestamp);
    //imageInfo["format"] = format;
    //imageInfo["data"] = base64Data;//图片原始数据,需要压缩后传输
    //Todo 发送图片数据
    //emit gShare.sigSent(Session::RequestString(11, "camera", "onImageInfoChanged", imageInfo));


    //二进制数据 发送
    quint8 invoke = share::ModuleName::camera;
    quint8 userID = QString(info.UserID).toInt();
    quint32  triggerID = pFrame->uTriggerId;
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out << invoke;
    out << userID;
    out << triggerID;

    //out << thumbnail.size() << thumbnail.format();
    //out.writeRawData(reinterpret_cast<const char*>(thumbnail.bits()), thumbnail.sizeInBytes());
    out << byteArray;
    qDebug() << "data.size" << bytes.size();
    emit gShare.sigSentBinary(bytes);
    return 0;
}

// 使用函数映射表来处理不同类型的参数
using SetParamFunc = std::function<dvpStatus(dvpHandle, dvpStr, const QJsonValue&)>;
const std::map<QString, SetParamFunc> ParamSetters = {
    {"enum", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
        return dvpSetEnumValue(handle, paramName, value.toInt());
    }},
    {"float", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
        return dvpSetFloatValue(handle, paramName, value.toDouble());
    }},
    {"int32", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
        return dvpSetInt32Value(handle, paramName, value.toInt());
    }},
    {"bool", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
        return dvpSetBoolValue(handle, paramName, value.toBool());
    }},
    {"string", [](dvpHandle handle, dvpStr paramName, const QJsonValue& value) {
        QByteArray ba = value.toString().toLocal8Bit();
        return dvpSetStringValue(handle, paramName, ba.data());
    }}
};

#include "public/serial/LeiShenLidarN10Plus.h"
QPointer<LeiShenLidarN10Plus> gAnotherSerial;//可在其类内使用智能独占指针 当对象被删除时,QPointer自动设为nullptr,QPointer只能用于QObject及其子类

bool DvpLineScanCamera::initialize() {
    qDebug() << "#DvpLineScanCamera: initialize";
    has_image_format = "bmp,jpeg,jpg,png,tiff,tif,gif,dat";//支持的图片格式
    gAnotherSerial = new LeiShenLidarN10Plus();
    //another = new LeiShenLidarN10Plus(nullptr,gShare.RegisterSettings->value(KEY_LIDAR_PORTNAME).toString());
    return scan();
}

DvpLineScanCamera::~DvpLineScanCamera() {
    delete gAnotherSerial;
    gAnotherSerial = nullptr;
}

Result DvpLineScanCamera::SetCameraConfig(const QJsonObject& config) {
    //qDebug() << "#DvpLineScanCamera: SetCamerasParams:" << params;
    QJsonObject general = config.value("general").toObject();
    if (general.contains("format")) g_image_format = general.value("format").toString();

    QJsonObject commonParams = config.value("params").toObject();
    QJsonObject taskParams = config.value("task").toObject();
    for (dvpUint32 i = 0; i < camera_scan_count; ++i) {
        auto& device = camera_info_array[i];
        if (!IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            qDebug() << "无效句柄,正在打开设备" << device.info.UserID;//会打印 m_pBase->Open  return:1
            resultDvp = dvpOpenByUserId(device.info.UserID, OPEN_NORMAL, &device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_WARNING(tr("#DvpLineScanCamera:Failed to open the camera"));
                continue;
            }
        }
        for (auto& key : commonParams.keys()) {
            QJsonObject param = commonParams.value(key).toObject();
            Result result = SetCameraParam(device.handle, param);//根据json文件重置参数
            if (!result) return result;
        }
        //更新设备个性化参数,只更新存在的设备
        if (taskParams.contains(device.info.UserID)) { //更新相机的设置的json信息
            device.path = taskParams[device.info.UserID].toObject()["path"].toString();
        }
        //相机默认存储路径
        if (device.path.isEmpty()) device.path = gShare.appPath + "/CAM" + device.info.UserID + "/";
        QDir dir(device.path);
        if (!dir.exists()) {
            if (!dir.mkpath(device.path)) {
                LOG_WARNING(tr("#DvpLineScanCamera:Failed to create directory ") + device.path);
                return Result::Failure("无法创建目录: " + device.path);
            }
        }
    }
    return Result(0, ("设置相机参数成功"));
}


Result DvpLineScanCamera::SetCameraParam(const dvpHandle& handle, const QJsonObject& param) {
    // 方法1：保持str在更长的作用域内
    std::string str = param["NodeName"].toString().toStdString();// 获取参数名
    if (str.empty()) {
        return Result::Failure("NodeName is empty");
    }
    dvpStr param_name = str.c_str(); // 只要str存在，指针就有效 保持str在此作用域内有效
    QString type = param["Type"].toString();// #根据类型设置参数 类型
    QJsonValue defaultValue = param["Default"];
    // 查找对应的设置函数
    auto it = ParamSetters.find(type);
    if (it != ParamSetters.end()) {
        // 调用对应的设置函数
        dvpStatus ret = it->second(handle, param_name, defaultValue);
        if (ret != DVP_STATUS_OK) {
            QString errMsg = QString("SetCameraParams NodeName:%1 Type:%2 error code:%3").arg(param_name).arg(type).arg(ret);
            LOG_WARNING(errMsg);
            return Result(int(ret), errMsg);
        }
    } else {
        QString errMsg = QString("Unsupported parameter type: %1 for parameter: %2").arg(type).arg(param["NodeName"].toString());
        LOG_WARNING(errMsg);
        return Result::Failure(errMsg);
    }
    return Result(0, ("设置相机参数成功"));
}

Result DvpLineScanCamera::SetLineScanMode(const dvpHandle& handle) {
    resultDvp = dvpSetBoolValue(handle, "LineTrigEnable", true);
    //Set frame trigger to disable
    resultDvp = dvpSetBoolValue(handle, "TriggerMode", false);

    //bool trigger_mode_enabled;
    //resultDvp = dvpGetTriggerState(handle, &trigger_mode_enabled);
    //if (resultDvp != DVP_STATUS_OK) {
    //    LOG_ERROR(tr("#DvpLineScanCamera:Get Trigger state fail!"));
    //} else {
    //    if (trigger_mode_enabled) {
    //        resultDvp = dvpSetTriggerState(handle, false);
    //        if (resultDvp != DVP_STATUS_OK) {
    //            LOG_ERROR(tr("#DvpLineScanCamera:Set Trigger state fail!"));
    //        }
    //    }
    //}

    //bool isLineTrigEnable;
    //resultDvp = dvpGetBoolValue(handle, "LineTrigEnable", &isLineTrigEnable);
    //if (resultDvp != DVP_STATUS_OK) {
    //    LOG_ERROR(tr("#DvpLineScanCamera:Get Bool Value LineTrigEnable fail!"));
    //} else {
    //    if (!isLineTrigEnable) {
    //        resultDvp = dvpSetBoolValue(handle, "LineTrigEnable", true);
    //        if (resultDvp != DVP_STATUS_OK) {
    //            LOG_ERROR(tr("#DvpLineScanCamera:Set Bool Value LineTrigEnable true fail!"));
    //        }
    //    }
    //}
    return Result();
}

Result DvpLineScanCamera::scan() {
    resultDvp = dvpRefresh(&camera_scan_count);// 获得当前能连接的相机数量
    if (resultDvp != DVP_STATUS_OK) {
        LOG_WARNING(tr("#DvpLineScanCamera:About Refresh fail!"));
        return Result::Failure("dvpRefresh error");
    }
    if (camera_scan_count != kDefaultCameraCount) {
        LOG_WARNING(tr("#DvpLineScanCamera: Current number of cameras: %1 Not the default number: %2").arg(camera_scan_count).arg(kDefaultCameraCount));
    }
    camera_id_list.clear();
    for (dvpUint32 i = 0; i < camera_scan_count; ++i) {
        // 逐个枚举出每个相机的信息
        auto& device = camera_info_array[i];
        resultDvp = dvpEnum(i, &device.info);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_ERROR(tr("#DvpLineScanCamera:About Enumerate fail! resultDvp:%1").arg(resultDvp));
        } else {
            device.name = device.info.FriendlyName;
            device.id = device.info.UserID;
            qDebug() << "#DvpLineScanCamera: scan " << device.name << device.id;
            //devicesIdList.append(QString::fromLocal8Bit(cameraInfoArray[i].info.FriendlyName));
            camera_id_list.append(device.id);
        }
    }
    return Result(0, ("扫描相机成功"));
}

Result DvpLineScanCamera::open() {
    qDebug() << QThread::currentThread() << "Result DvpLineScanCamera::open()";
    for (dvpUint32 i = 0; i < camera_scan_count; ++i) {
        auto& device = camera_info_array[i];
        //for (auto& device : cameraInfoArray) {
        if (IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        resultDvp = dvpOpenByUserId(device.info.UserID, OPEN_NORMAL, &device.handle);
        //resultDvp = dvpOpenByName(device.info.FriendlyName, OPEN_NORMAL, &device.handle);
        //resultDvp = dvpOpenByName(device.info.FriendlyName, OPEN_OFFLINE, &device.handle);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING(tr("#DvpLineScanCamera:Failed to open the camera"));
            continue;
        }
        // 线扫模式使能配置 Set line trigger to enable 需要关闭触发使能,打开线扫模式使能
        //SetLineScanMode(device.handle);//设置线扫模式使能
        //打开不用设置参数,参数步骤单独设置
        //Result result = SetCamerasParams(device.handle);//根据json文件重置参数
        //if (!result) return result;
        //成员函数不能直接用作回调函数指针，因为成员函数隐含一个 this 指针参数，与普通函数的签名不同
        //1.静态成员函数 2.使用全局函数 3.使用 std::function 和 lambda（如果API支持）
        //resultDvp = dvpRegisterStreamCallback(device.handle, CallbackAcquiringVideoStream, STREAM_EVENT_FRAME_THREAD, &device);//单独的线程
        resultDvp = dvpRegisterStreamCallback(device.handle, AcquiringImageCallback, STREAM_EVENT_FRAME_THREAD, &device);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING(tr("#DvpLineScanCamera:%1 Failed to Register the stream callback!").arg(device.info.FriendlyName));
        }
        dvpStreamState state;
        resultDvp = dvpGetStreamState(device.handle, &state);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING(tr("#DvpLineScanCamera:Get the stream state fail!"));
            continue;
        }
        if (state == STATE_STARTED) {
            resultDvp = dvpStop(device.handle);
        }
    }

    if (!gAnotherSerial->open(gShare.RegisterSettings->value(CAMERA_KEY_PORTNAME).toString())) {
        return Result::Failure("打开雷达串口失败");
    };
    return Result(0, ("打开相机成功"));
}

Result DvpLineScanCamera::close() {
    qDebug() << QThread::currentThread() << "Result DvpLineScanCamera::close()";
    for (dvpUint32 i = 0; i < camera_scan_count; ++i) {
        auto& device = camera_info_array[i];
        //for (auto& device : cameraInfoArray) {
        if (!IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        dvpStreamState state;
        resultDvp = dvpGetStreamState(device.handle, &state);
        // 如果图像正在采集，则调用Stop停止采集
        if (state == STATE_STARTED) {
            resultDvp = dvpStop(device.handle);
        }
        //注销视频流回调函数
        resultDvp = dvpUnregisterStreamCallback(device.handle, AcquiringImageCallback, STREAM_EVENT_FRAME_THREAD, &device);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING(tr("#DvpLineScanCamera:Failed to Unregister the stream callback!camera:%1").arg(device.info.FriendlyName));
        }
        //resultDvp = dvpSaveConfig(device.handle, 0);
        resultDvp = dvpClose(device.handle);//关闭相机后，相机句柄将不再可用。但是再次打开相机时，又可能会得到相同的句柄值 
        device.handle = 0;
    }
    gAnotherSerial->close();
    return Result(true, ("关闭相机成功"));
}

Result DvpLineScanCamera::start() {
    for (dvpUint32 i = 0; i < camera_scan_count; ++i) {
        auto& device = camera_info_array[i];
        //for (auto& device : cameraInfoArray) {
        if (!IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        resultDvp = dvpGetStreamState(device.handle, &device.state);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_ERROR(tr("#DvpLineScanCamera:Get the stream state fail!camera:%1").arg(device.info.FriendlyName));
            continue;
        }
        if (device.state == STATE_STOPED) {
            //开始采集 设置线扫模式使能
            //SetLineScanMode(device.handle);//硬件只支持设置线扫模式使能
            resultDvp = dvpStart(device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_ERROR(tr("#DvpLineScanCamera:Failed to Start the video stream!camera:%1,Code:%2").arg(device.info.FriendlyName).arg(resultDvp));
                return Result(int(resultDvp), tr("%1Failed to start acquisition").arg(device.info.FriendlyName));
            }
        }
    }
    return Result(true, ("开始采集成功"));
}

Result DvpLineScanCamera::stop() {
    for (dvpUint32 i = 0; i < camera_scan_count; ++i) {
        auto& device = camera_info_array[i];
        //for (auto& device : cameraInfoArray) {
        if (!IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        resultDvp = dvpGetStreamState(device.handle, &device.state);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_ERROR(tr("#DvpLineScanCamera:Failed get the stream state fail!camera:%1").arg(device.info.FriendlyName));
            continue;
        }
        if (device.state == STATE_STARTED) {
            resultDvp = dvpStop(device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_ERROR(tr("#DvpLineScanCamera:Failed stop the video stream!camera:%1").arg(device.info.FriendlyName));
            }
        }
    }
    return Result(true, ("停止采集成功"));
}


Result DvpLineScanCamera::triggerFire() {
    for (dvpUint32 i = 0; i < camera_scan_count; ++i) {
        auto& device = camera_info_array[i];
        bool triggerState;
        resultDvp = dvpGetTriggerState(device.handle, &triggerState);
        if (resultDvp != DVP_STATUS_FUNCTION_INVALID) {
            resultDvp = dvpTriggerFire(device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_WARNING("#DvpLineScanCamera:Trigger fire fail!");
                return Result(11, ("相机软触发失败"));
            }
        }
    }
    return Result(0, ("相机软触发成功"));
}

void DvpLineScanCamera::start(const Session& session) {
    gShare.on_send(start(), session);

}

void DvpLineScanCamera::stop(const Session& session) {
    gShare.on_send(stop(), session);
}

Result DvpLineScanCamera::OnStarted(CallbackResult callback) {
    Result result = start();
    if (callback) { callback(result); }
    return result;
}

Result DvpLineScanCamera::OnStopped(CallbackResult callback) {
    Result result = stop();
    if (callback) { callback(result); }
    return result;
}

QJsonObject DvpLineScanCamera::GetDeviceIdList() const {
    QJsonObject obj;
    obj["camera_id_list"] = camera_id_list.join(",");
    obj["serial_id_list"] = gAnotherSerial->GetAvailablePorts().join(",");
    return obj;
}


Result DvpLineScanCamera::Property() {
    if (IsValidHandle(camera_info_array[active_index].handle)) {
#ifdef Q_OS_WIN32
        //this就是要获取句柄的窗体的类名；
        //dvpShowPropertyModalDialog(m_handle, (HWND)this->winId());
        dvpShowPropertyModalDialog(camera_info_array[active_index].handle, nullptr);
#endif
        return Result(0, ("显示成功"));
    }
    return Result(11, ("显示失败"));
}