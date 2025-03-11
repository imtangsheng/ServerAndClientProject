#include "pch.h"
#include <QFile>


QString struCameraInfo::format = gSouth.kImageFormat[0];
const QString CameraConfigFileName = "config/camera_params.json";
const QString CAMERA_PATH[8] = {
    "E:/Test/CAM1/",
    "E:/Test/CAM2/",
    "E:/Test/CAM3/",
    "E:/Test/CAM4/",
    "E:/Test/CAM5/",
    "E:/Test/CAM6/",
    "E:/Test/CAM7/",
    "E:/Test/CAM8/"
};

// 全局回调函数
static INT AcquiringImageCallback(dvpHandle handle, dvpStreamEvent event, void* pContext, dvpFrame* pFrame, void* pBuffer) {
    qDebug() << QThread::currentThreadId() << "采集回调函数" << handle << event;
    // 通过pContext访问设备对象
    struCameraInfo* device = static_cast<struCameraInfo*>(pContext);
    device->AcquiringImage(pFrame, pBuffer);
    return 0;
}

CameraSDK::CameraSDK(QObject* parent) : QObject(parent) {
    initialize();
    // 预先浏览相机设备
    scan();
    //if(scan()) open();
}

void CameraSDK::test() {
    qDebug() << "CameraSDK::test()";
    //根据指定分辨率抓取并保存图像 
    dvpHandle handle = cameraInfoArray->handle;
    dvpUint32 resoulutionModeSelect;
    dvpGetResolutionModeSel(handle, &resoulutionModeSelect);
    dvpRegion roi;
    dvpGetRoi(handle, &roi);
    dvpUint32 timeout = 1000;
    dvpStr filePath = "E:/Test/CAM1/1.jpeg";
    dvpInt32 quality = 100;
    dvpStatus state = dvpCaptureFile(handle, resoulutionModeSelect, &roi, timeout, filePath, quality);
    if(state != DVP_STATUS_OK)
    {
        qDebug() << "dvpCaptureFile error:"<<state;//-53 使用不当
    } 
}

Result CameraSDK::LoadConfigFromFile(const QString& jsonFilePath) {
    Result result;
    QString absolutePath;
    result = gSouth.FindFilePath(jsonFilePath, absolutePath);
    if (!result || absolutePath.isEmpty()) {
        LOG_WARNING(result.message);
        return result;
    }
    //获取相机配置文件的json数据
    result = gSouth.ReadJsonFile(absolutePath, cameraConfigJson);
    if (!result || cameraConfigJson.isEmpty()) {
        LOG_WARNING(result.message);
        return result;
    }
    //获取相机信息设置
    cameraInfoJson = cameraConfigJson["cameraInfoJson"].toObject();
    //获取相机参数,使用共同的参数
    cameraParamsJson = cameraConfigJson["camera_params"].toObject();
    return true;
}

Result CameraSDK::SaveConfigToFile(const QString& jsonFilePath) {
    QString relativePaths = jsonFilePath;
    // 检测文件后缀是否为json
    if (relativePaths.isEmpty() || !relativePaths.endsWith(".json", Qt::CaseInsensitive)) {
        relativePaths = CameraConfigFileName;
    }
    Result result;

    //获取相机信息
    // 创建JSON对象
    QJsonObject infoJson;
    // 遍历所有设备信息并保存
    for (const auto& device : cameraInfoArray) {
        infoJson[device.name] = device.ToJson();  // 假设有getId方法获取设备ID
    }
    cameraConfigJson["cameraInfoJson"] = infoJson;
    result = gSouth.WriteJsonFile(relativePaths, cameraConfigJson);
    if (!result) {
        LOG_WARNING(result.message);
    }
    return result;  // 返回结果
}

void CameraSDK::initialize() {
    qDebug() << "#CameraSDK: initialize相机SDK初始化";
    LoadConfigFromFile(CameraConfigFileName);
}

Result CameraSDK::SetCamerasParams(const dvpHandle& handle) {
    //根据json更新参数相机的设置
    //#遍历并设置相机参数
    Result result;
    for (const QString& key : cameraParamsJson.keys()) {
        QJsonObject param_camera = cameraParamsJson[key].toObject();
        result = SetCamerasParams(handle, param_camera);
        if (!result) return result;
    }
    return result;
}

Result CameraSDK::SetCamerasParams(const dvpHandle& handle, const QJsonObject& param) {
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
            return Result(int(ret),errMsg);
        }
    } else {
        QString errMsg = QString("Unsupported parameter type: %1 for parameter: %2").arg(type).arg(param["NodeName"].toString());
        LOG_WARNING(errMsg);
        return Result::Failure(errMsg);
    }
    return Result(0, tr("设置相机参数成功"));
}

Result CameraSDK::scan() {
    resultDvp = dvpRefresh(&devicesCount);// 获得当前能连接的相机数量
    if (resultDvp != DVP_STATUS_OK) {
        LOG_WARNING(tr("#CameraSDK:About Refresh fail!"));
        return Result::Failure("dvpRefresh error");
    }
    if (devicesCount != CurrentCameraCount) {
        LOG_WARNING(tr("#CameraSDK:当前设备数量:%1 不是默认的相机个数:%2").arg(devicesCount).arg(CurrentCameraCount));
    }
    devicesNamesList.clear();
    for (dvpUint32 i = 0; i < devicesCount; ++i) {
        // 逐个枚举出每个相机的信息
        resultDvp = dvpEnum(i, &cameraInfoArray[i].info);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_ERROR(tr("#CameraSDK:About Enumerate fail! resultDvp:%1").arg(resultDvp));
        } else {
            QString name = cameraInfoArray[i].info.FriendlyName;
            qDebug() << "#CameraSDK: scan " << name;
            //devicesNamesList.append(QString::fromLocal8Bit(cameraInfoArray[i].info.FriendlyName));
            devicesNamesList.append(name);
            if (cameraInfoJson.contains(name)) { //更新相机的设置的json信息
                cameraInfoArray[i].update(cameraInfoJson[name].toObject());
            }
            //相机默认存储路径
            if (cameraInfoArray[i].path.isEmpty()) cameraInfoArray[i].path = CAMERA_PATH[i];
        }
    }
    return Result(0, tr("扫描相机成功"));
}

Result CameraSDK::open() {
    qDebug() << QThread::currentThread() << "Result CameraSDK::open()";
    for (dvpUint32 i = 0; i < devicesCount; ++i) {
        auto& device = cameraInfoArray[i];
        //for (auto& device : cameraInfoArray) {
        if (IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        //status = dvpOpenByUserId(CStringA(strName), OPEN_NORMAL, &m_handle);

        resultDvp = dvpOpenByName(device.info.FriendlyName, OPEN_NORMAL, &device.handle);
        //resultDvp = dvpOpenByName(device.info.FriendlyName, OPEN_OFFLINE, &device.handle);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING(tr("#CameraSDK:Open the camera fail!"));
            continue;
        }
        Result result = SetCamerasParams(device.handle);//根据json文件重置参数
        if (!result) return result;
        //成员函数不能直接用作回调函数指针，因为成员函数隐含一个 this 指针参数，与普通函数的签名不同
        //1.静态成员函数 2.使用全局函数 3.使用 std::function 和 lambda（如果API支持）
        //resultDvp = dvpRegisterStreamCallback(device.handle, CallbackAcquiringVideoStream, STREAM_EVENT_FRAME_THREAD, &device);//单独的线程
        resultDvp = dvpRegisterStreamCallback(device.handle, AcquiringImageCallback, STREAM_EVENT_FRAME_THREAD, &device);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING(tr("#CameraSDK:注册视频流回调函数Register the stream callback fail!camera:%1").arg(device.info.FriendlyName));
        }
        dvpStreamState state;
        resultDvp = dvpGetStreamState(device.handle, &state);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING(tr("#CameraSDK:Get the stream state fail!"));
            continue;
        }
        if (state == STATE_STARTED) {
            resultDvp = dvpStop(device.handle);
        }
    }
    return Result(0, tr("打开相机成功"));
}

Result CameraSDK::close() {
    qDebug() << QThread::currentThread() << "Result CameraSDK::close()";
    for (dvpUint32 i = 0; i < devicesCount; ++i) {
        auto& device = cameraInfoArray[i];
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
            LOG_WARNING(tr("#CameraSDK:注销视频流回调函数Unregister the stream callback fail!camera:%1").arg(device.info.FriendlyName));
        }
        //resultDvp = dvpSaveConfig(device.handle, 0);
        resultDvp = dvpClose(device.handle);//关闭相机后，相机句柄将不再可用。但是再次打开相机时，又可能会得到相同的句柄值 
        device.handle = 0;
    }
    return Result(true, tr("关闭相机成功"));
}

Result CameraSDK::start() {
    for (dvpUint32 i = 0; i < devicesCount; ++i) {
        auto& device = cameraInfoArray[i];
        //for (auto& device : cameraInfoArray) {
        if (!IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        resultDvp = dvpGetStreamState(device.handle, &device.state);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_ERROR(tr("#CameraSDK:Get the stream state fail!camera:%1").arg(device.info.FriendlyName));
            continue;
        }
        if (device.state == STATE_STOPED) {
            bool bTrigStatus;
            resultDvp = dvpGetTriggerState(device.handle, &bTrigStatus);
            if (resultDvp != DVP_STATUS_FUNCTION_INVALID) {
            //	// 在启动视频流之前先设置为触发模式 在触发出图和连续出图模式之间切换,看不懂
                resultDvp = dvpSetTriggerState(device.handle, isSoftTrigger ? true : false);
            	if (resultDvp != DVP_STATUS_OK) {
            		LOG_WARNING(tr("#CameraSDK:Set status of trigger fail!"));
            	}
            }
            resultDvp = dvpStart(device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_ERROR(tr("#CameraSDK:Start the video stream fail!camera:%1,Code:%2").arg(device.info.FriendlyName).arg(resultDvp));
                return Result(int(resultDvp), tr("%1开始采集失败").arg(device.info.FriendlyName));
            }
        }
    }
    return Result(true, tr("开始采集成功"));
}

Result CameraSDK::stop() {
    for (dvpUint32 i = 0; i < devicesCount; ++i) {
        auto& device = cameraInfoArray[i];
        //for (auto& device : cameraInfoArray) {
        if (!IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        resultDvp = dvpGetStreamState(device.handle, &device.state);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_ERROR(tr("#CameraSDK:Get the stream state fail!camera:%1").arg(device.info.FriendlyName));
            continue;
        }
        if (device.state == STATE_STARTED) {
            resultDvp = dvpStop(device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_ERROR(tr("#CameraSDK:Stop the video stream fail!camera:%1").arg(device.info.FriendlyName));
            }
        }
    }
    return Result(true, tr("停止采集成功"));
}


Result CameraSDK::Property() {
    if (IsValidHandle(cameraInfoArray[activeIndex].handle)) {
#ifdef Q_OS_WIN32
        //this就是要获取句柄的窗体的类名；
        //dvpShowPropertyModalDialog(m_handle, (HWND)this->winId());
        dvpShowPropertyModalDialog(cameraInfoArray[activeIndex].handle, nullptr);
#endif
        return Result(0, tr("显示成功"));
    }
    return Result(11, tr("显示失败"));
}

Result CameraSDK::triggerFire() {
    bool triggerState;
    resultDvp = dvpGetTriggerState(cameraInfoArray[activeIndex].handle, &triggerState);
    if (resultDvp != DVP_STATUS_FUNCTION_INVALID) {
        resultDvp = dvpTriggerFire(cameraInfoArray[activeIndex].handle);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING("#CameraSDK:Trigger fire fail!");
            return Result(11, tr("相机软触发失败"));
        }
    }
    return Result(0, tr("相机软触发成功"));
}

Result CameraSDK::prepare(const Session& session) {
    //触发使能
    bool triggerState = true;
    resultDvp = dvpSetTriggerState(cameraInfoArray[activeIndex].handle, &triggerState);
    return Result(session.id, tr("敬请期待"));
}

Result CameraSDK::slotDispRate() {
    dvpStatus status;
    dvpFrameCount       FrameCount;
    if (IsValidHandle(cameraInfoArray[activeIndex].handle)) {
        // 更新帧率信息
        status = dvpGetFrameCount(cameraInfoArray[activeIndex].handle, &FrameCount);
        if (status != DVP_STATUS_OK) {
            qWarning() << ("#CameraSDK:Get frame count fail!");
        }
        QString strFrameInfo = tr("采集帧数:%1 采集帧率:%2").arg(FrameCount.uFrameCount).arg(FrameCount.fFrameRate);
        qDebug() << strFrameInfo;
        return Result(true, strFrameInfo);
    }
    return Result::Failure("Invalid handle");
}