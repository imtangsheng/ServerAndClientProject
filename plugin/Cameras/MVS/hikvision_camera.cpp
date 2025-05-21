#include "hikvision_camera.h"
#include<QImage>
#include<QBuffer>

static MV_SAVE_IAMGE_TYPE g_enSaveImageType{ MV_Image_Jpeg };
static bool IsColor(MvGvspPixelType enType) {
    switch (enType) {
    case PixelType_Gvsp_BGR8_Packed:
    case PixelType_Gvsp_YUV422_Packed:
    case PixelType_Gvsp_YUV422_YUYV_Packed:
    case PixelType_Gvsp_BayerGR8:
    case PixelType_Gvsp_BayerRG8:
    case PixelType_Gvsp_BayerGB8:
    case PixelType_Gvsp_BayerBG8:
    case PixelType_Gvsp_BayerGB10:
    case PixelType_Gvsp_BayerGB10_Packed:
    case PixelType_Gvsp_BayerBG10:
    case PixelType_Gvsp_BayerBG10_Packed:
    case PixelType_Gvsp_BayerRG10:
    case PixelType_Gvsp_BayerRG10_Packed:
    case PixelType_Gvsp_BayerGR10:
    case PixelType_Gvsp_BayerGR10_Packed:
    case PixelType_Gvsp_BayerGB12:
    case PixelType_Gvsp_BayerGB12_Packed:
    case PixelType_Gvsp_BayerBG12:
    case PixelType_Gvsp_BayerBG12_Packed:
    case PixelType_Gvsp_BayerRG12:
    case PixelType_Gvsp_BayerRG12_Packed:
    case PixelType_Gvsp_BayerGR12:
    case PixelType_Gvsp_BayerGR12_Packed:
    case PixelType_Gvsp_BayerRBGG8:
    case PixelType_Gvsp_BayerGR16:
    case PixelType_Gvsp_BayerRG16:
    case PixelType_Gvsp_BayerGB16:
    case PixelType_Gvsp_BayerBG16:
        return true;
    default:
        return false;
    }
}

static bool IsMono(MvGvspPixelType enType) {
    switch (enType) {
    case PixelType_Gvsp_Mono10:
    case PixelType_Gvsp_Mono10_Packed:
    case PixelType_Gvsp_Mono12:
    case PixelType_Gvsp_Mono12_Packed:
    case PixelType_Gvsp_Mono14:
    case PixelType_Gvsp_Mono16:
        return true;
    default:
        return false;
    }
}
static bool IsHBPixelFormat(MvGvspPixelType ePixelType) {
    switch (ePixelType) {
    case PixelType_Gvsp_HB_Mono8:
    case PixelType_Gvsp_HB_Mono10:
    case PixelType_Gvsp_HB_Mono10_Packed:
    case PixelType_Gvsp_HB_Mono12:
    case PixelType_Gvsp_HB_Mono12_Packed:
    case PixelType_Gvsp_HB_Mono16:
    case PixelType_Gvsp_HB_RGB8_Packed:
    case PixelType_Gvsp_HB_BGR8_Packed:
    case PixelType_Gvsp_HB_RGBA8_Packed:
    case PixelType_Gvsp_HB_BGRA8_Packed:
    case PixelType_Gvsp_HB_RGB16_Packed:
    case PixelType_Gvsp_HB_BGR16_Packed:
    case PixelType_Gvsp_HB_RGBA16_Packed:
    case PixelType_Gvsp_HB_BGRA16_Packed:
    case PixelType_Gvsp_HB_YUV422_Packed:
    case PixelType_Gvsp_HB_YUV422_YUYV_Packed:
    case PixelType_Gvsp_HB_BayerGR8:
    case PixelType_Gvsp_HB_BayerRG8:
    case PixelType_Gvsp_HB_BayerGB8:
    case PixelType_Gvsp_HB_BayerBG8:
    case PixelType_Gvsp_HB_BayerRBGG8:
    case PixelType_Gvsp_HB_BayerGB10:
    case PixelType_Gvsp_HB_BayerGB10_Packed:
    case PixelType_Gvsp_HB_BayerBG10:
    case PixelType_Gvsp_HB_BayerBG10_Packed:
    case PixelType_Gvsp_HB_BayerRG10:
    case PixelType_Gvsp_HB_BayerRG10_Packed:
    case PixelType_Gvsp_HB_BayerGR10:
    case PixelType_Gvsp_HB_BayerGR10_Packed:
    case PixelType_Gvsp_HB_BayerGB12:
    case PixelType_Gvsp_HB_BayerGB12_Packed:
    case PixelType_Gvsp_HB_BayerBG12:
    case PixelType_Gvsp_HB_BayerBG12_Packed:
    case PixelType_Gvsp_HB_BayerRG12:
    case PixelType_Gvsp_HB_BayerRG12_Packed:
    case PixelType_Gvsp_HB_BayerGR12:
    case PixelType_Gvsp_HB_BayerGR12_Packed:
        return true;
    default:
        return false;
    }
}

// 保存文件
static bool SaveToFile(unsigned char* pData, int nLen,const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR(QObject::tr("Failed to open file: %1").arg(filename));
        return false;
    }
    //可使用缓冲区提高写入性能
    qint64 written = file.write(reinterpret_cast<char*>(pData), nLen);
    file.close();
    return written == nLen;
}

static void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser) {
    // 记录开始时间
    QDateTime startTime = QDateTime::currentDateTime();
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << QThread::currentThread() << "采集回调函数调用";
    // 将两个32位的无符号整数连接起来，形成一个64位的整数  
    quint64 combined = (quint64)pFrameInfo->nDevTimeStampHigh << 32 | pFrameInfo->nDevTimeStampLow;
    qDebug() << "enPixelType" << pFrameInfo->enPixelType << "nFrameNum" << pFrameInfo->nFrameNum << "nTriggerIndex " << pFrameInfo->nTriggerIndex
        << "nHostTimeStamp" << pFrameInfo->nHostTimeStamp<< QDateTime::fromMSecsSinceEpoch(pFrameInfo->nHostTimeStamp) << "timestamp" << QDateTime::fromMSecsSinceEpoch(combined);

    // 通过pUser访问设备对象
    MvCamera* device = static_cast<MvCamera*>(pUser);
    QString image_path = QString("%1/%2#%3.%4")
        .arg(device->path)
        .arg(pFrameInfo->nFrameNum)
        .arg(pFrameInfo->nHostTimeStamp)
        .arg(g_image_format);
    qDebug() << "图片路径" << image_path;

	//需要旋转270度
    MV_CC_ROTATE_IMAGE_PARAM stRotateParam{};
    // 设置源图像参数
    stRotateParam.enPixelType = pFrameInfo->enPixelType;
    stRotateParam.nWidth = pFrameInfo->nWidth;
    stRotateParam.nHeight = pFrameInfo->nHeight;
    stRotateParam.pSrcData = pData;//原始图像数据
    stRotateParam.nSrcDataLen = pFrameInfo->nFrameLen;//输入数据长度
    //分配目标缓冲区
    unsigned int nConvertDataSize{ 0 };
    if (IsColor(pFrameInfo->enPixelType)) {
        nConvertDataSize = pFrameInfo->nWidth * pFrameInfo->nHeight * 3;
    } else if (IsMono(pFrameInfo->enPixelType)) {
        nConvertDataSize = pFrameInfo->nWidth * pFrameInfo->nHeight;
    } else {
        qWarning() << "不支持的像素格式";
        return;
    }
    stRotateParam.nDstBufSize = nConvertDataSize;//提供的输出缓冲区大小
    //设置为270度旋转
    stRotateParam.enRotationAngle = MV_IMAGE_ROTATE_270;
    MV_CC_RotateImage(device->handle, &stRotateParam);
    //数据交换
    pFrameInfo->nWidth = stRotateParam.nWidth;
    pFrameInfo->nHeight = stRotateParam.nHeight;
    pFrameInfo->nFrameLen = stRotateParam.nDstBufLen;
    pData = stRotateParam.pDstBuf;

    //非RAW格式
    if (g_enSaveImageType != MV_Image_Undefined) {
        //保存图像到文件接口
        MV_CC_IMAGE stImg;
        MV_CC_SAVE_IMAGE_PARAM stSaveParams;
        memset(&stSaveParams, 0, sizeof(MV_CC_SAVE_IMAGE_PARAM));
        memset(&stImg, 0, sizeof(MV_CC_SAVE_IMAGE_PARAM));
        stImg.enPixelType = pFrameInfo->enPixelType;
        stImg.nHeight = pFrameInfo->nHeight;
        stImg.nWidth = pFrameInfo->nWidth;
        stImg.nImageBufLen = pFrameInfo->nFrameLenEx;
        stImg.pImageBuf = pData;
        //保存的格式此处需要一个转换,还有不支持raw格式
        stSaveParams.enImageType = g_enSaveImageType;
        stSaveParams.iMethodValue = 1;
        stSaveParams.nQuality = 99;

        unsigned int ret = MV_CC_SaveImageToFileEx2(device->handle, &stImg, &stSaveParams, image_path.toLocal8Bit().constData());
        if (MV_OK != ret) {
            LOG_ERROR(QObject::tr("[#ImageCallBackEx]MV_CC_SaveImageToFileEx2 failed:0x%1").arg(ret, 0, 16));
            return;
        }
    } else {
        // 检查是否为HB模式的像素格式
        if (IsHBPixelFormat(pFrameInfo->enPixelType)) {
            // HB格式需要解码
            MV_CC_HB_DECODE_PARAM stDecodeParam = { 0 };
            stDecodeParam.pSrcBuf = pData;
            stDecodeParam.nSrcLen = pFrameInfo->nFrameLen;
            // 分配目标缓冲区
            unsigned int nPayloadSize = pFrameInfo->nWidth * pFrameInfo->nHeight * 3;
            uchar* pDstBuf = new uchar[nPayloadSize];
            stDecodeParam.pDstBuf = pDstBuf;
            stDecodeParam.nDstBufSize = nPayloadSize;
            if (MV_OK == MV_CC_HB_Decode(device->handle, &stDecodeParam)) { // HB解码
                SaveToFile(stDecodeParam.pDstBuf, stDecodeParam.nDstBufLen, image_path);
            } else {
                LOG_ERROR(QObject::tr("[#ImageCallBackEx]MV_CC_HB_Decode failed"));
            }
            delete[] pDstBuf;
        } else {
            SaveToFile(pData, pFrameInfo->nFrameLen, image_path);
        }
    }//图片保存

    /* 图像数据转发QImage::Format_Grayscale8*/
    //检测图片格式
    QImage image_test(image_path);
    if (!image_test.isNull()) {
        qDebug() << "image_test format"<<image_test.format();
    }

    // 1. 将原始图像转为QImage
    const uchar* data = static_cast<const uchar*>(pData);
    const int width = pFrameInfo->nWidth;
    const int height = pFrameInfo->nHeight;
    qDebug() << "width" << width << "height" << height << "format" << g_enSaveImageType;
    QImage image;
    MvGvspPixelType pixelType = pFrameInfo->enPixelType;
    if ((pixelType & 0xFF000000) == MV_GVSP_PIX_MONO) {
        qDebug() << "单色（灰度或 Bayer）";
        image = QImage(data, width, height, QImage::Format_Grayscale8);// 创建深拷贝避免数据竞争 .cppy()
    }else if ((pixelType & 0xFF000000) == MV_GVSP_PIX_COLOR) {
        qDebug() << "彩色（RGB、YUV 等）";
        image = QImage(data, width, height, QImage::Format_RGB888);
    }
    // 2. 极限缩放（缩小到1%面积，约10%长宽）
    const double SCALE_FACTOR = 0.01; // 5%线性缩放 
    QSize minSize = image.size() * SCALE_FACTOR;
    minSize = minSize.expandedTo(QSize(64, 64)); // 保证最小32x32
    qDebug() << "image.size" << image.size() << image.sizeInBytes() << "minSize"<<minSize;
    //缩略图
    QImage thumbnail = image.scaled(minSize,
        Qt::IgnoreAspectRatio,//意味着允许图像变形
        Qt::FastTransformation); // 速度优先
    qDebug() << "thumbnail.size" << thumbnail.size() << thumbnail.sizeInBytes();
    //tips:此处开始下一帧触发处理
    // 4. 动态压缩至目标大小（<50KB）
    QByteArray compressedData;//压缩数据
    QBuffer buffer(&compressedData);
    int quality = 40; // 初始低质量
    thumbnail.save(&buffer, "JPG", quality);//自定义格式无压缩,时间慢,改为jpg最大压缩,最快151毫秒bmp,151M, jpg是26M 547毫秒
    qDebug() << "compressedData.size" << compressedData.size() << "quality" << quality;
    QByteArray& byteArray = compressedData;
    //二进制数据 发送
    quint8 invoke = share::ModuleName::camera;
    quint8 userID = QString(device->id).toInt();
    quint32  triggerID = pFrameInfo->nFrameNum;
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out << invoke;
    out << userID;
    out << triggerID;
    out << byteArray;
    qDebug() << "data.size" << bytes.size();
    emit gShare.sigSentBinary(bytes);

    int elapsed = startTime.msecsTo(QDateTime::currentDateTime());
    qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "回调函数执行时间:" << elapsed << "毫秒";
    return;
}


HiKvisionCamera::~HiKvisionCamera() {
    qDebug() << "[#HiKvisionCamera]~HiKvisionCamera()";
    for (auto& device : camera_list) {
        if (device->handle != nullptr) {
            nRet = MV_CC_DestroyHandle(device->handle);
            if (MV_OK != nRet) {
                LOG_ERROR(tr("[#HiKvisionCamera]MV_CC_DestroyHandle failed:0x%1").arg(nRet, 0, 16));
            }
            device->handle = nullptr;
        }
    }
    // ch:反初始化SDK | en:Finalize SDK
    MV_CC_Finalize();
}

#include "public/serial/ActiveSerial.h"
ActiveSerial* gAnotherSerial = nullptr;//可使用智能指针 替代手动管理生命周期

bool HiKvisionCamera::initialize() {
    nRet = MV_CC_Initialize();
    if (MV_OK != nRet) {
        LOG_ERROR(tr("[#HiKvisionCamera]MV_CC_Initialize failed! nRet:0x%1").arg(nRet, 0, 16));
        return false;
    }
    has_image_format = "jpeg,bmp,png,tiff,raw";//支持的图片格式
    gAnotherSerial = new ActiveSerial();
    return scan();
}

Result HiKvisionCamera::SetCameraConfig(const QJsonObject& config) {
    //qDebug() << "#DvpLineScanCamera: SetCamerasParams:" << params;
    QJsonObject general = config.value("general").toObject();
    if (general.contains("format")) g_image_format = general.value("format").toString();

    QJsonObject commonParams = config.value("params").toObject();
    QJsonObject taskParams = config.value("task").toObject();
    for (auto& device : camera_list) {
        if (device->handle == nullptr) {
            LOG_WARNING(tr("[#HiKvisionCamera]%1 Handle is NULL!").arg(device->id));
            nRet = MV_CC_CreateHandle(&device->handle, device->info);
            if (MV_OK != nRet) {
                LOG_ERROR(tr("[#HiKvisionCamera]%1 Create Handle fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
                continue;
            }
        }
        //设置相机参数
        for (auto& key : commonParams.keys()) {
            QJsonObject param = commonParams.value(key).toObject();
            Result result = SetCameraParam(device->handle, param);//根据json文件重置参数
            if (!result) return result;
        }
        //更新设备个性化参数,只更新存在的设备
        if (taskParams.contains(device->id)) { //更新相机的设置的json信息
            device->path = taskParams[device->id].toObject()["path"].toString();
        }
        //相机默认存储路径
        if (device->path.isEmpty()) device->path = gShare.appPath + "/CAM" + device->id + "/";
        //创建存储路径
        QDir dir(device->path);
        if (!dir.exists()) {
            if (!dir.mkpath(device->path)) {
                LOG_WARNING(tr("[#HiKvisionCamera]Failed to create directory ") + device->path);
                return Result::Failure("无法创建目录: " + device->path);
            }
        }
    }
    return Result();
}

Result HiKvisionCamera::scan() {
    // ch:枚举设备 | en:Enum device
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
    nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
    if (MV_OK != nRet) {
        LOG_ERROR(tr("[#HiKvisionCamera]MV_CC_EnumDevices failed:0x%1").arg(nRet, 0, 16));
        return Result::Failure("MV_CC_EnumDevices error");
    }
    if (stDeviceList.nDeviceNum > 0) {
        camera_scan_count = stDeviceList.nDeviceNum; camera_list.clear(); camera_id_list.clear();
        for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++) {
            MvCamera* device = new MvCamera();//使用动态大小还是固定大小?
            camera_list.append(device);
            device->info = stDeviceList.pDeviceInfo[i];
            MV_CC_DEVICE_INFO* pMVDevInfo = stDeviceList.pDeviceInfo[i];
            if (NULL == pMVDevInfo) {
                LOG_ERROR(tr("[#HiKvisionCamera]pDeviceInfo is null"));
                break;
            }
            //可获取相机信息
            if (pMVDevInfo->nTLayerType == MV_GIGE_DEVICE) {
                int nIp1 = ((pMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
                int nIp2 = ((pMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
                int nIp3 = ((pMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
                int nIp4 = (pMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);
                // ch:打印当前相机ip和用户自定义名字 | en:print current ip and user defined name
                printf("CurrentIp: %d.%d.%d.%d\n", nIp1, nIp2, nIp3, nIp4);
                qDebug() << "[#]UserDefinedName" << pMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName;
                qDebug() << "[#]SerialNumber" << pMVDevInfo->SpecialInfo.stGigEInfo.chSerialNumber;
                qDebug() << "[#]chModelName" << pMVDevInfo->SpecialInfo.stGigEInfo.chModelName;
                device->id = QString::fromUtf8(reinterpret_cast<char*>(pMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName));
            } else if (pMVDevInfo->nTLayerType == MV_USB_DEVICE) {
                qDebug() << "[#]UserDefineName" << QString((const char*)pMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
                printf("UserDefinedName: %s\n", pMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
                printf("Serial Number: %s\n", pMVDevInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
                printf("Device Number: %d\n\n", pMVDevInfo->SpecialInfo.stUsb3VInfo.nDeviceNumber);

                device->id = QString::fromUtf8(reinterpret_cast<char*>(pMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName));
            } else {
                LOG_WARNING("Not support.");
            }
            camera_id_list.append(device->id);
        }//循环读取相机设备信息
    } else {
        LOG_WARNING(tr("[#HiKvisionCamera]Find No Devices"));
        return Result::Failure("Find No Devices");
    }
    return Result();
}

Result HiKvisionCamera::open() {
    for (auto& device : camera_list) {
        // ch:选择设备并创建句柄 | en:Select device and create handle
        nRet = MV_CC_CreateHandle(&device->handle, device->info);
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Create Handle fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
            continue;
        }
        // ch:打开设备 | en:Open device
        nRet = MV_CC_OpenDevice(device->handle);
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Open Device fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));//0x80000203 无访问权限,是否被占用
            continue;
        }
    }
    //if (!another->open(gShare.RegisterSettings->value(CAMERA_KEY_PORTNAME).toString())) {
    //    return Result::Failure("打开相机串口失败");
    //};
    return Result();
}

Result HiKvisionCamera::close() {
    for (auto& device : camera_list) {
        if (device->handle == nullptr) continue;//无效句柄
        // ch:关闭设备 | en:Close device
        nRet = MV_CC_CloseDevice(device->handle);
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Close Device fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
            continue;
        }
        // ch:销毁句柄 | en:Destroy handle
        nRet = MV_CC_DestroyHandle(device->handle);
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Destroy Handle fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
            continue;
        }
        device->handle = nullptr;
    }
    gAnotherSerial->close();
    return Result();
}

Result HiKvisionCamera::start() {
    for (auto& device : camera_list) {
        if (device->handle == nullptr) continue;//无效句柄
        SetTriggerMode(device->handle);// ch:设置触发模式为off | en:Set trigger mode as off
        // ch:注册抓图回调 | en:Register image callback
        nRet = MV_CC_RegisterImageCallBackEx(device->handle, ImageCallBackEx, device);
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Register Image CallBack fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
            continue;
        }
        // ch:开始取流 | en:Start grab image
        nRet = MV_CC_StartGrabbing(device->handle);
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Start Grabbing fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
            continue;
        }
    }
    return Result();
}

Result HiKvisionCamera::stop() {
    for (auto& device : camera_list) {
        if (device->handle == nullptr) continue;//无效句柄
        // ch:停止取流 | en:Stop grab image
        nRet = MV_CC_StopGrabbing(device->handle);
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Stop Grabbing fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
            continue;
        }
        // ch:注销抓图回调 | en:Unregister image callback
        nRet = MV_CC_RegisterImageCallBackEx(device->handle, NULL, NULL);
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Unregister Image CallBack fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
            continue;
        }

    }
    return Result();
}

Result HiKvisionCamera::triggerFire() {
    for (auto& device : camera_list) {
        if (device->handle == nullptr) continue;
        nRet = MV_CC_SetCommandValue(device->handle, "TriggerSoftware");
        if (MV_OK != nRet) {
            LOG_ERROR(tr("[#HiKvisionCamera]%1 Trigger Command fail! nRet:0x%2").arg(device->id).arg(nRet, 0, 16));
            continue;
        }
    }
    return Result();
}

void HiKvisionCamera::start(const Session& session) {
    start();//没有返回值的
    gAnotherSerial->start(session);

}

void HiKvisionCamera::stop(const Session& session) {
    stop();
    gAnotherSerial->stop(session);
}

Result HiKvisionCamera::OnStarted(CallbackResult callback) {
    start();
    return gAnotherSerial->OnStarted(callback);
}

Result HiKvisionCamera::OnStopped(CallbackResult callback) {
    stop();
    return gAnotherSerial->OnStopped(callback);
}

QJsonObject HiKvisionCamera::GetDeviceIdList() const {
    QJsonObject obj;
    obj["camera_id_list"] = camera_id_list.join(",");
    obj["serial_id_list"] = gAnotherSerial->GetAvailablePorts().join(",");
    return obj;
}


bool HiKvisionCamera::SetImageFormat(const QString& format) {
    // 根据文件后缀选择保存格式
    if (format == "jpg" || format == "jpeg") {
        g_enSaveImageType = MV_Image_Jpeg;
    } else if (format == "png") {
        g_enSaveImageType = MV_Image_Png;
    } else if (format == "bmp") {
        g_enSaveImageType = MV_Image_Bmp;
    } else if (format == "tiff" || format == "tif") {
        g_enSaveImageType = MV_Image_Tif;
    } else if (format == "raw") {
        g_enSaveImageType = MV_Image_Undefined;
    } else {
        LOG_ERROR(tr("Unsupported format:%1").arg(format));
        return false;
    }
    g_image_format = format;
    return true;
}

// 使用函数映射表来处理不同类型的参数
using SetParamFunc = std::function<int(void*, const char*, const QJsonValue&)>;
const std::map<QString, SetParamFunc> ParamSetters = {
    {"enum", [](void* handle, const char* paramName, const QJsonValue& value) {
        return MV_CC_SetEnumValue(handle, paramName, value.toInt());
    }},
    {"float", [](void* handle, const char* paramName, const QJsonValue& value) {
        return MV_CC_SetFloatValue(handle, paramName, value.toDouble());
    }},
    {"int32", [](void* handle, const char* paramName, const QJsonValue& value) {
        return MV_CC_SetIntValue(handle, paramName, value.toInt());
    }},
    {"bool", [](void* handle, const char* paramName, const QJsonValue& value) {
        return MV_CC_SetBoolValue(handle, paramName, value.toBool());
    }},
    {"string", [](void* handle, const char* paramName, const QJsonValue& value) {
        QByteArray ba = value.toString().toLocal8Bit();
        return MV_CC_SetStringValue(handle, paramName, ba.data());
    }}
};

Result HiKvisionCamera::SetCameraParam(void* handle, const QJsonObject& param) {
    // 方法1：保持str在更长的作用域内
    std::string str = param["NodeName"].toString().toStdString();// 获取参数名
    if (str.empty()) {
        return Result::Failure("NodeName is empty");
    }
    const char* param_name = str.c_str(); // 只要str存在，指针就有效 保持str在此作用域内有效
    QString type = param["Type"].toString();// #根据类型设置参数 类型
    QJsonValue defaultValue = param["Default"];
    // 查找对应的设置函数
    auto it = ParamSetters.find(type);
    if (it != ParamSetters.end()) {
        // 调用对应的设置函数
        unsigned int ret = it->second(handle, param_name, defaultValue);
        if (ret != MV_OK) {
            QString errMsg = QString("SetCameraParams NodeName:%1 Type:%2 error code:0x%3").arg(param_name).arg(type).arg(ret, 0, 16);
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

Result HiKvisionCamera::SetTriggerMode(void* handle) {
    // ch:设置触发模式为off | en:Set trigger mode as off
    nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 1);
    if (MV_OK != nRet) {
        LOG_ERROR(tr("[#HiKvisionCamera]Set Trigger Mode fail! nRet:0x%1").arg(nRet, 0, 16));
        return Result::Failure("Set Trigger Mode fail!");
    }
    return Result();
}

