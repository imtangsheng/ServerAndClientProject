#include <QFile>
QString struCameraInfo::format = gSouth.kImageFormat[0];

// 全局回调函数
static INT AcquiringImageCallback(dvpHandle handle, dvpStreamEvent event, void* pContext, dvpFrame* pFrame, void* pBuffer) {
    //qDebug() << QThread::currentThreadId() << "采集回调函数调用 handle event" << handle << event;
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
    dvpStatus state = dvpCaptureFile(handle, resoulutionModeSelect, &roi, timeout, filePath, quality); //已经弃用
    if (state != DVP_STATUS_OK) {
        qDebug() << "dvpCaptureFile error:" << state;//-53 使用不当
    }
}


void CameraSDK::initialize() {
    qDebug() << "#CameraSDK: initialize";
}

Result CameraSDK::SetCameraConfig(const QJsonObject& config) {
    //qDebug() << "#CameraSDK: SetCamerasParams:" << params;
    QJsonObject general = config.value("general").toObject();
    if(general.contains("format")) struCameraInfo::format = general.value("format").toString(); 

    QJsonObject commonParams = config.value("params").toObject();
    QJsonObject taskParams = config.value("task").toObject();
    for (dvpUint32 i = 0; i < CameraActualTotal; ++i) {
        auto& device = cameraInfoArray[i];
        if (!IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            qDebug() << "无效句柄,正在打开设备" << device.info.UserID;//会打印 m_pBase->Open  return:1
            resultDvp = dvpOpenByUserId(device.info.UserID, OPEN_NORMAL, &device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_WARNING(tr("#CameraSDK:Failed to open the camera"));
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
        if (device.path.isEmpty()) device.path = gSouth.appDirPath + "/CAM" + device.info.UserID + "/";
        QDir dir(device.path);
        if (!dir.exists()) {
            if (!dir.mkpath(device.path)) {
                LOG_WARNING(tr("#CameraSDK:Failed to create directory ") + device.path);
                return Result::Failure("无法创建目录: " + device.path);
            }
        }
    }
    return Result(0, ("设置相机参数成功"));
}


Result CameraSDK::SetCameraParam(const dvpHandle& handle, const QJsonObject& param) {
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

Result CameraSDK::SetLineScanMode(const dvpHandle& handle) {
    resultDvp = dvpSetBoolValue(handle, "LineTrigEnable", true);
    //Set frame trigger to disable
    resultDvp = dvpSetBoolValue(handle, "TriggerMode", false);

    //bool isTriggerState;
    //resultDvp = dvpGetTriggerState(handle, &isTriggerState);
    //if (resultDvp != DVP_STATUS_OK) {
    //    LOG_ERROR(tr("#CameraSDK:Get Trigger state fail!"));
    //} else {
    //    if (isTriggerState) {
    //        resultDvp = dvpSetTriggerState(handle, false);
    //        if (resultDvp != DVP_STATUS_OK) {
    //            LOG_ERROR(tr("#CameraSDK:Set Trigger state fail!"));
    //        }
    //    }
    //}

    //bool isLineTrigEnable;
    //resultDvp = dvpGetBoolValue(handle, "LineTrigEnable", &isLineTrigEnable);
    //if (resultDvp != DVP_STATUS_OK) {
    //    LOG_ERROR(tr("#CameraSDK:Get Bool Value LineTrigEnable fail!"));
    //} else {
    //    if (!isLineTrigEnable) {
    //        resultDvp = dvpSetBoolValue(handle, "LineTrigEnable", true);
    //        if (resultDvp != DVP_STATUS_OK) {
    //            LOG_ERROR(tr("#CameraSDK:Set Bool Value LineTrigEnable true fail!"));
    //        }
    //    }
    //}
    return Result();
}

Result CameraSDK::scan() {
    resultDvp = dvpRefresh(&CameraActualTotal);// 获得当前能连接的相机数量
    if (resultDvp != DVP_STATUS_OK) {
        LOG_WARNING(tr("#CameraSDK:About Refresh fail!"));
        return Result::Failure("dvpRefresh error");
    }
    if (CameraActualTotal != CurrentCameraCount) {
        LOG_WARNING(tr("#CameraSDK: Current number of cameras: %1 Not the default number: %2").arg(CameraActualTotal).arg(CurrentCameraCount));
    }
    devicesIdList.clear();
    for (dvpUint32 i = 0; i < CameraActualTotal; ++i) {
        // 逐个枚举出每个相机的信息
        resultDvp = dvpEnum(i, &cameraInfoArray[i].info);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_ERROR(tr("#CameraSDK:About Enumerate fail! resultDvp:%1").arg(resultDvp));
        } else {
            QString name = cameraInfoArray[i].info.FriendlyName;
            QString id = cameraInfoArray[i].info.UserID;
            qDebug() << "#CameraSDK: scan " << name << id;
            //devicesIdList.append(QString::fromLocal8Bit(cameraInfoArray[i].info.FriendlyName));
            devicesIdList.append(id);
        }
    }
    return Result(0, ("扫描相机成功"));
}

Result CameraSDK::open() {
    qDebug() << QThread::currentThread() << "Result CameraSDK::open()";
    for (dvpUint32 i = 0; i < CameraActualTotal; ++i) {
        auto& device = cameraInfoArray[i];
        //for (auto& device : cameraInfoArray) {
        if (IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        resultDvp = dvpOpenByUserId(device.info.UserID, OPEN_NORMAL, &device.handle);
        //resultDvp = dvpOpenByName(device.info.FriendlyName, OPEN_NORMAL, &device.handle);
        //resultDvp = dvpOpenByName(device.info.FriendlyName, OPEN_OFFLINE, &device.handle);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_WARNING(tr("#CameraSDK:Failed to open the camera"));
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
            LOG_WARNING(tr("#CameraSDK:%1 Failed to Register the stream callback!").arg(device.info.FriendlyName));
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
    return Result(0, ("打开相机成功"));
}

Result CameraSDK::close() {
    qDebug() << QThread::currentThread() << "Result CameraSDK::close()";
    for (dvpUint32 i = 0; i < CameraActualTotal; ++i) {
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
            LOG_WARNING(tr("#CameraSDK:Failed to Unregister the stream callback!camera:%1").arg(device.info.FriendlyName));
        }
        //resultDvp = dvpSaveConfig(device.handle, 0);
        resultDvp = dvpClose(device.handle);//关闭相机后，相机句柄将不再可用。但是再次打开相机时，又可能会得到相同的句柄值 
        device.handle = 0;
    }
    return Result(true, ("关闭相机成功"));
}

Result CameraSDK::start() {
    for (dvpUint32 i = 0; i < CameraActualTotal; ++i) {
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
            //开始采集 设置线扫模式使能
            //SetLineScanMode(device.handle);//硬件只支持设置线扫模式使能
            resultDvp = dvpStart(device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_ERROR(tr("#CameraSDK:Failed to Start the video stream!camera:%1,Code:%2").arg(device.info.FriendlyName).arg(resultDvp));
                return Result(int(resultDvp), tr("%1Failed to start acquisition").arg(device.info.FriendlyName));
            }
        }
    }
    return Result(true, ("开始采集成功"));
}

Result CameraSDK::stop() {
    for (dvpUint32 i = 0; i < CameraActualTotal; ++i) {
        auto& device = cameraInfoArray[i];
        //for (auto& device : cameraInfoArray) {
        if (!IsValidHandle(device.handle)) {//有效句柄,说明已经打开过
            continue;
        }
        resultDvp = dvpGetStreamState(device.handle, &device.state);
        if (resultDvp != DVP_STATUS_OK) {
            LOG_ERROR(tr("#CameraSDK:Failed get the stream state fail!camera:%1").arg(device.info.FriendlyName));
            continue;
        }
        if (device.state == STATE_STARTED) {
            resultDvp = dvpStop(device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_ERROR(tr("#CameraSDK:Failed stop the video stream!camera:%1").arg(device.info.FriendlyName));
            }
        }
    }
    return Result(true, ("停止采集成功"));
}


Result CameraSDK::Property() {
    if (IsValidHandle(cameraInfoArray[activeIndex].handle)) {
#ifdef Q_OS_WIN32
        //this就是要获取句柄的窗体的类名；
        //dvpShowPropertyModalDialog(m_handle, (HWND)this->winId());
        dvpShowPropertyModalDialog(cameraInfoArray[activeIndex].handle, nullptr);
#endif
        return Result(0, ("显示成功"));
    }
    return Result(11, ("显示失败"));
}

Result CameraSDK::triggerFire() {
    for (dvpUint32 i = 0; i < CameraActualTotal; ++i) {
        auto& device = cameraInfoArray[i];
        bool triggerState;
        resultDvp = dvpGetTriggerState(device.handle, &triggerState);
        if (resultDvp != DVP_STATUS_FUNCTION_INVALID) {
            resultDvp = dvpTriggerFire(device.handle);
            if (resultDvp != DVP_STATUS_OK) {
                LOG_WARNING("#CameraSDK:Trigger fire fail!");
                return Result(11, ("相机软触发失败"));
            }
        }
    }
    return Result(0, ("相机软触发成功"));
}

Result CameraSDK::prepare(const Session& session) {
    //触发使能
    bool triggerState = true;
    resultDvp = dvpSetTriggerState(cameraInfoArray[activeIndex].handle, &triggerState);
    return Result(session.id, ("敬请期待"));
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
        //采集帧数:%1 采集帧率:%2
        QString strFrameInfo = tr("Frames:%1 Rate:%2").arg(FrameCount.uFrameCount).arg(FrameCount.fFrameRate);
        qDebug() << strFrameInfo;
        return Result(true, strFrameInfo);
    }
    return Result::Failure("Invalid handle");
}