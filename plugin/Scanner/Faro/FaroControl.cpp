#include<QTimer>
#include "FaroControl.h"
using namespace faro;
#include "api/faro.ls.sdk.tlh"

static IiQLicensedInterfaceIfPtr liPtr(nullptr); //许可
static IScanCtrlSDKPtr scanCtrl(nullptr);// = static_cast<IScanCtrlSDKPtr>(liPtr);

// 全局回调函数（静态变量）
static ScanCompletedCallback g_scanCompletedCallback = nullptr;
#pragma region _IScanCtrlSDKEvents 事件接收器类 - 实现了_IScanCtrlSDKEvents接口，用于接收扫描SDK的事件

class CScanEventSink : public _IScanCtrlSDKEvents
{
private:
    LONG m_cRef;  // COM对象引用计数，用于管理对象生命周期

public:
    // 构造函数：初始化引用计数为1
    CScanEventSink() : m_cRef(1) {}

    // IUnknown 接口实现 - COM对象必须实现的基础接口
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        // 检查客户端请求的接口是否被支持
        if (riid == IID_IUnknown ||          // 基础COM接口
            riid == IID_IDispatch ||         // 自动化接口
            riid == __uuidof(_IScanCtrlSDKEvents)) // 扫描事件接口
        {
            *ppv = this;      // 返回当前对象指针
            AddRef();         // 增加引用计数
            return S_OK;      // 成功
        }
        *ppv = NULL;
        return E_NOINTERFACE; // 不支持请求的接口
    }

    // 增加引用计数 - 线程安全的原子操作
    STDMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement(&m_cRef);
    }

    // 减少引用计数 - 当计数为0时自动删除对象
    STDMETHODIMP_(ULONG) Release() {
        LONG lRef = InterlockedDecrement(&m_cRef);
        if (lRef == 0)
            delete this;  // 自动释放内存
        return lRef;
    }

    // IDispatch 接口实现 - 用于支持自动化和脚本调用

    // 返回类型信息数量 - 这里返回0表示不提供类型信息
    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo) {
        *pctinfo = 0;
        return S_OK;
    }

    // 获取类型信息 - 未实现
    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo) {
        return E_NOTIMPL;
    }

    // 获取方法/属性的调度ID - 未实现
    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames,
        UINT cNames, LCID lcid, DISPID* rgDispId) {
        return E_NOTIMPL;
    }

    // 最重要的方法 - 处理事件调用的核心函数
    // 当COM对象触发事件时，会通过这个方法通知事件接收器
    STDMETHODIMP Invoke(DISPID dispIdMember,    // 方法的调度ID
        REFIID riid,             // 接口ID
        LCID lcid,               // 地区设置
        WORD wFlags,             // 调用标志
        DISPPARAMS* pDispParams, // 参数信息
        VARIANT* pVarResult,     // 返回值
        EXCEPINFO* pExcepInfo,   // 异常信息
        UINT* puArgErr)          // 错误参数索引
    {
        // 根据调度ID分发到具体的事件处理函数
        qDebug() << "COM对象触发事件,事件调用Id:" << dispIdMember;
        switch (dispIdMember) {
        case 0x1: // scanCompleted 事件的调度ID
            return scanCompleted();
        default:
            return DISP_E_MEMBERNOTFOUND; // 未找到对应的方法
        }
    }

    // 实现具体的扫描完成事件处理
    STDMETHODIMP scanCompleted() {
        // 输出调试信息到Qt调试控制台
        LOG_INFO(QObject::tr("扫描仪COM对象触发事件"));
        // 调用全局回调函数（如果已设置）
        if (g_scanCompletedCallback) {
            g_scanCompletedCallback();
        }
        return S_OK;
    }
};

// 事件连接管理类 - 负责建立和管理与COM对象的事件连接
class CEventConnection
{
private:
    IConnectionPointContainer* m_pContainer;    // 连接点容器接口
    IConnectionPoint* m_pConnectionPoint;       // 具体的连接点接口
    CScanEventSink* m_pEventSink;              // 事件接收器对象
    DWORD m_dwCookie;                          // 连接标识符

public:
    // 构造函数：初始化所有指针为NULL
    CEventConnection() : m_pContainer(NULL), m_pConnectionPoint(NULL),
        m_pEventSink(NULL), m_dwCookie(0) {
    }

    // 析构函数：确保断开所有连接
    ~CEventConnection() {
        Disconnect();
    }

    // 连接到COM对象的事件源
    HRESULT Connect(IUnknown* pSource) {
        HRESULT hr;

        // 步骤1：获取连接点容器接口
        // 连接点容器是COM对象提供事件连接服务的接口
        hr = pSource->QueryInterface(IID_IConnectionPointContainer,
            (void**)&m_pContainer);
        if (FAILED(hr))
            return hr;

        // 步骤2：查找特定事件接口的连接点
        // 每种事件接口都有对应的连接点
        hr = m_pContainer->FindConnectionPoint(__uuidof(_IScanCtrlSDKEvents),
            &m_pConnectionPoint);
        if (FAILED(hr)) {
            m_pContainer->Release(); // 释放已获取的接口
            m_pContainer = NULL;
            return hr;
        }

        // 步骤3：创建事件接收器实例
        m_pEventSink = new CScanEventSink();
        if (!m_pEventSink) {
            Disconnect();
            return E_OUTOFMEMORY; // 内存分配失败
        }

        // 步骤4：建立事件连接
        // Advise方法将事件接收器注册到连接点，返回连接标识符 
        // 一旦连接建立（即 Advise 成功），COM 对象会在事件发生时（如扫描完成）主动调用事件接收器（CScanEventSink）的 Invoke 方法，并根据 DISPID 分发到相应的事件处理函数（如 scanCompleted）。
        hr = m_pConnectionPoint->Advise(m_pEventSink, &m_dwCookie);
        if (FAILED(hr)) {
            Disconnect();
            return hr;
        }

        return S_OK; // 连接成功
    }

    // 断开事件连接并清理资源
    void Disconnect() {
        // 断开事件连接
        if (m_pConnectionPoint && m_dwCookie != 0) {
            m_pConnectionPoint->Unadvise(m_dwCookie); // 取消事件通知
            m_dwCookie = 0;
        }

        // 释放连接点接口
        if (m_pConnectionPoint) {
            m_pConnectionPoint->Release();
            m_pConnectionPoint = NULL;
        }

        // 释放连接点容器接口
        if (m_pContainer) {
            m_pContainer->Release();
            m_pContainer = NULL;
        }

        // 释放事件接收器对象
        if (m_pEventSink) {
            m_pEventSink->Release(); // 减少引用计数，可能触发对象删除
            m_pEventSink = NULL;
        }
    }
};
#pragma endregion


FaroControl::FaroControl(QObject* parent)
    : QObject{ parent } {
    qDebug() << "[#FaroControl]构造函数:" << QThread::currentThread();
    try {
        // 初始化COM组件
        if (FAILED(CoInitialize(nullptr))) { throw tr("[#Faro]初始化COM组件错误"); }
        // 创建并验证许可接口
        liPtr = IiQLicensedInterfaceIfPtr(__uuidof(ScanCtrlSDK));
        if (!liPtr) {throw tr("[#Faro]创建并验证许可接口错误");}
        liPtr->License = LicenseCode;
        scanCtrl = static_cast<IScanCtrlSDKPtr>(liPtr);
        if(!scanCtrl){throw tr("[#Faro]创建SDK接口错误");}
    } catch (const QString& error) {
        LOG_ERROR(tr("#[致命错误]Faro:法如扫描初始化错误: %1").arg(error));
        //- 致命错误
        qFatal() << tr("#[致命错误]Faro:法如扫描初始化错误: %1").arg(error);//输出后程序会立即终止运行
    }

    // 创建事件处理器并连接 
    static CEventConnection eventConnection;// 事件连接管理器
    // 事件连接会在析构函数中自动断开
    HRESULT hr = eventConnection.Connect(scanCtrl);// IUnknown* m_pScanSDK;              // 扫描SDK COM对象
    if (SUCCEEDED(hr)) {
        qDebug() << "事件监听已成功建立";
            // 运行消息循环等待事件 COM 事件通知依赖于 Windows 消息机制（通常通过内部窗口或消息泵）。当 COM 对象触发事件时，它会向消息队列发送消息，DispatchMessage 确保这些消息被分发到 CScanEventSink 的 Invoke 方法。
            //MSG msg;//GetMessage(&msg, NULL, 0, 0)：从消息队列中获取一条消息。如果队列为空，它会阻塞直到有消息到达（除非指定了超时或特定消息过滤）
            //if (GetMessage(&msg, NULL, 0, 0)) {
            //    TranslateMessage(&msg);//TranslateMessage(&msg)：将虚拟键消息转换为字符消息（主要用于键盘输入，这里可能无关）。
            //    DispatchMessage(&msg);//DispatchMessage(&msg)：将消息分发给相应的窗口过程或 COM 事件处理机制。
            //}
    } else {
        LOG_ERROR(tr("建立事件监听失败: %1").arg(hr));
    }
    
    //ip = "192.168.43.1";//无线
    ip = "172.17.9.20";//有线
    //ip = "192.168.1.20";//测试

    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &FaroControl::onDirectoryChanged);
    connect(&watcherPreview, &QFileSystemWatcher::directoryChanged,this, &FaroControl::addPreviewFile);

    //测试预览
    //startMonitoring("../test/PointCloud");
}

FaroControl::~FaroControl() {
    qDebug() << "[#FaroControl]::~FaroControl析构执行";
    if (scanCtrl) {
        //scanCtrl->Release();//With these pointers, you don’t have  to  call  Release  at  the end  of  usage.  Instead,  you  have  to  assign NULL  to  the  smart  interface pointer. 
        scanCtrl = nullptr;
    }
    if (liPtr) {
        //liPtr->Release();
        liPtr = nullptr;
    }
    CoUninitialize();
}

bool FaroControl::isConnect() {
    return scanCtrl->Connected;/* 0 == FALSE, -1 == TRUE */
}

int FaroControl::Connect() {
    qDebug() << "[#Faro]尝试连接" << ip <<"[时间]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    scanCtrl->ScannerIP = faro::FaroString(ip);
    //scanCtrl->clearExceptions();
    ret = scanCtrl->connect(); //可能的结果:0, 2, 4 ⚠️注意,该函数会冻结程序运行,无线连接21秒,并且在有线网络时候,其返回2,结果不准,在约24秒后通过连接查询会连接上
    qDebug() << "FaroControl::connect 连接结果" << ret;
    return ret;
}

Result FaroControl::SetParameters(QJsonObject param) {
    /* ScanMode 可以有四个值：
    0 固定灰色 设置 StationaryGrey 以进行普通球面灰度扫描。
    1 固定颜色 设置 StationaryColor 以记录带颜色的球面扫描
    2 螺旋灰 将 HelicalGrey 设置为使用 TTL 接口以螺旋模式进行扫描。
    3 螺旋CANGrey 将 HelicalCANGrey 设置为使用 CAN 通信以螺旋模式进行扫描。
    */
    qDebug() << "扫描仪将设置参数:" << param;
    QString path = param.value(JSON_OriginalScanDir).toString();
    if (path.isEmpty()) {
        if (gTaskFileInfo == nullptr) {
            return Result::Failure(tr("目录参数为空").arg(path));
        }
        path = gTaskFileInfo->path + "/" + kTaskDirPointCloudName;
    }
    QDir dir(path);
    // 检查目录是否存在
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {// 递归创建所有需要的目录层级
            return Result::Failure(tr("目录%1创建失败").arg(path));
        }
    }
    Q_ASSERT(dir.exists());
    scanCtrl->_ScanMode = HelicalCanGrey;
    scanCtrl->_StorageMode = SMRemote;//SMRemote: Store scans on the remote computer (the computer the SDK is running on).
    scanCtrl->ScanFileNumber = 1;
    scanCtrl->ScanBaseName = L"Scan";
    //BSTR RemoteScanStoragePath = SysAllocString(reinterpret_cast<const OLECHAR*>(dir.absolutePath().utf16()));
    scanCtrl->RemoteScanStoragePath = faro::FaroString(dir.absolutePath());//(gTaskFileInfo->path + "/" + kTaskDirPointCloudName);

    int resolution = param.value(Json_Resolution).toInt(4);// Only the following values are permitted: 1, 2, 4, 5, 8, 10, 16, 20, 32
    scanCtrl->Resolution = resolution;
    int MeasurementRate = param.value(Json_MeasurementRate).toInt(8);
    scanCtrl->MeasurementRate = MeasurementRate;//Possible values: 1, 2, 4, 8. 法如操作界面的质量属性 quality
    scanCtrl->NoiseCompression = 1;//Possible values: 1, 2, 4. 
    //scanCtrl->HorizontalAngleMin = 0;
    //scanCtrl->HorizontalAngleMax = 360;
    scanCtrl->VerticalAngleMin = -60;//In ScanMode HelicalGrey and HelicalCANGrey this parameter will automatically be set to 0° and cannot be changed
    scanCtrl->VerticalAngleMax = 90;

    /*In  ScanMode StationaryGrey NumCols  will  automatically  be  adjusted  every  time,
    when Resolution, HorizontalAngleMin, or HorizontalAngleMax  has  changed.So  this  parameter should be changed at last
    设置要记录的扫描线数。当达到此数字时，扫描仪将停止扫描。也可以选择将扫描数据流自动拆分为多个文件
    */
    int num_cols = param.value(Json_NumCols).toInteger(2000000);
    scanCtrl->NumCols = num_cols; // 2000000; //In ScanMode HelicalGrey changes on Resolution or the scan area will have no effect on NumCols.
    int SplitAfterLines = param.value(Json_SplitAfterLines).toInt(5000);
    scanCtrl->SplitAfterLines = SplitAfterLines;//The minimum number is 100. Smaller scan files are not supported.
    ret = scanCtrl->syncParam();// By calling syncParam they get synchronized with the scanner 
    qInfo() << "扫描仪参数设置执行结果" << ret << "任务当前状态:" << gTaskState << "任务希望状态:"<<TaskState::TaskState_Running;//应该是 running
    if (ret == faro::OK && gTaskState != TaskState::TaskState_Waiting) startMonitoring(path);
    return ret;
}

int FaroControl::ScanStart() {
    /* After having received the start signal the laser sensor needs approx. 10 scan columns to warm up. In this time the recorded scan data will be unusable
    * 收到启动信号后，激光传感器需要大约 10 个扫描色谱柱以预热。在此期间，记录的扫描数据将无法使用。
    * 请注意，在螺旋扫描模式下，记录的扫描点没有完整的 3D信息，并且必须使用额外的定位信息进行扩展（例如来自里程表或 GPS 的定位信息）。
    */
    //scanCtrl->clearExceptions();//Clears all exceptions on the scanner.This function is automatically called at the beginning of scanStart.
    //QDir dir(QString(scanCtrl->GetRemoteScanStoragePath()));
    //// 检查目录是否存在
    //if (!dir.exists()) {
    //    if (!dir.mkpath(".")) {// 递归创建所有需要的目录层级
    //        return Result::Failure(tr("目录%1创建失败").arg(dir.absolutePath()));
    //    }
    //}
    ret = scanCtrl->startScan();//0, 1, 3, 4  约等待8秒才可以触发录制的功能,否则点击无效
    LOG_INFO(tr("扫描仪开始扫描执行结果:%1").arg(ret));
    ret.lastError = tr("扫描仪开始扫描失败, 请检查设备状态");
    return ret;
}

int FaroControl::ScanRecord() {
    ret = scanCtrl->recordScan();//0,3 Starts scan data recording when scanning in helical CAN mode.
    HelicalRecordingStatus status{ HRSUnknown };//HRSUnknown = 0,HRSPaused = 1, HRSRecording = 2
    int count = 50;
    while (Result(ret.value)) {
        ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
        qDebug() << "#FaroControl::ScanRecord 查询结果" << ret << "和当前录制状态1暂停 2录制中:" << status;
        if (status == HRSRecording) { 
            if (!callbackStartedPtr.isNull() && *callbackStartedPtr) (*callbackStartedPtr)();
            break; 
        }
        if (count-- < 0) {
            qWarning() << "#FaroControl::ScanRecord 执行录制失败" << ret << "和当前录制状态1暂停 2录制中:" << status;
            ret.Set(status);//未知查询不到默认为执行成功
        }
    }
    return ret;
}

//return: 0, 3 Pauses scan data recording when scanning on helical CAN mode. Restart scan data recording with recordScan.
int FaroControl::ScanPause() {
    HelicalRecordingStatus status{ HRSUnknown };
    ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    qDebug() << "执行结果" << ret << status;
    qDebug() << "正在暂停";
    ret = scanCtrl->pauseScan();//需要等待生效
    ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    qDebug() << "执行结果" << ret;
    ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    qDebug() << "暂停执行结果查询" << ret << status;
    return ret;
}

int FaroControl::ScanStop() {
    int percent = scanCtrl->ScanProgress;
    int currentNumCols = scanCtrl->NumCols * percent / 100;
    qDebug() << "[#Scanner]停止扫描 进度:" << percent << "%" << currentNumCols;
    /*通过调整线数不会有效
    int stopNumCols = (currentNumCols / scanCtrl->SplitAfterLines + 1)* scanCtrl->SplitAfterLines;
    //scanCtrl->PutNumCols(stopNumCols);//umCols 必须由用户设置，并且在更改分辨率等其他参数时不会自动调整。
    //ret = scanCtrl->syncParam();// By calling syncParam they get synchronized with the scanner 1541 返回值不知道
    qDebug() << "调整线数执行结果" << ret << stopNumCols;
    */
    
    ret = scanCtrl->stopScan();//0,1, 3
    qDebug() << "扫描仪停止执行结果" << ret;
    switch (ret) {
    case faro::OK:
        return 0;
    case faro::BUSY:
        LOG_WARNING(tr("扫描仪停止任务执行忙, 请手动控制停止(浏览器强制停止),并检查设备状态"));
        emit onScanStateChanged(StateEvent::Failed);
        break;
    case faro::TIMEOUT:
        LOG_WARNING(tr("扫描仪停止任务执行超时, 请确认并检查设备状态"));
        emit onScanStateChanged(StateEvent::Stopped);
        break;
    case faro::NOT_CONNECTED:
        LOG_WARNING(tr("扫描仪停止任务执行超时, 请确认并检查设备连接状态"));
        emit onScanStateChanged(StateEvent::Timeout);
        break;
    default:
        ret.lastError = tr("扫描仪停止任务失败, 请检查设备状态");
        break;
    }
    return ret;
}

int FaroControl::GetScanPercent() {
    return scanCtrl->ScanProgress;
}

int FaroControl::shutdown()
{
    //http://192.168.43.1/lswebapi/operations/shutdown
    return scanCtrl->shutDown();//0, 3 
}


Result FaroControl::OnStarted(const CallbackResult& callback) {
    if(!isConnect()){
        ret.value = -1;
        ret.lastError = tr("扫描仪未连接");
        if (callback) callback(ret.value,ret.lastError);
        return Result(ret.value,ret.lastError);
    }
    callbackStartedPtr = QSharedPointer<ScanCompletedCallback>::create([=]() { //WaitStarted
        qDebug() << "[#Scanner]扫描启动完成,执行状态码"<<ret <<"callback:";
        ret.lastError = tr("扫描仪录制任务启动, 请检查设备状态");
        emit onScanStateChanged(StateEvent::Started);
        LOG_INFO(tr("扫描仪已启动, 录制数据中..."));
        if (callback) {
            callback(ret, ret.lastError);
        }
        callbackStartedPtr.reset();
        });

    ret = ScanStart();
    if (ret != faro::OK) {
        ret.lastError=tr("扫描仪开始采集任务失败, 请检查设备状态");
        return Result(ret.value, ret.lastError);
    }
    emit onScanStateChanged(StateEvent::PreStart);
    LOG_INFO(tr("扫描仪已启动,请等待10秒后开始采集数据"));
    //此处如果不使用CAN指令触发,则使用定时器9秒后开始录制(8秒多一点预启动完成)
    return true;
}

Result FaroControl::OnStopped(const CallbackResult& callback) {
    g_scanCompletedCallback = [this, &callback]() { //等待结束
        qDebug() << "[#Scanner]扫描停止采集完成";
        ret.lastError = tr("扫描仪停止采集, 请检查设备状态");
        emit onScanStateChanged(StateEvent::Stopped);
        LOG_INFO(tr("扫描仪停止采集"));
        if (callback) callback(ret, ret.lastError);
        g_scanCompletedCallback = nullptr;
        };
    ret = ScanStop();

    return true;
}

void FaroControl::onDirectoryChanged(const QString& path) {
    /* 获取当前文件列表
    * isNextStopToggled 标志表示完成当前的文件分块后,立马停止采集,表示需要采集一个完整的 fls文件
    * 实时预览需要等待采集完整的一个 fls文件,才能进行预览,即读取解析的是上一个文件的内容
    * 
    */
    QDir dir(path);
    // 只监控 .txt 和 .log 文件
    QStringList filters;
    filters << "*.fls";
    QFileInfoList currentFiles = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);//QDir::Time 时间排序
    if (this->isNextStopToggled) {
        if (ScanStop() == faro::OK) {
            this->isNextStopToggled = false;
            stopMonitoring();
            QString lastFilePath = currentFiles.last().absoluteFilePath();
            qInfo() << "停止扫描成功,15秒后删除文件:" << lastFilePath;
            LOG_INFO(tr("扫描仪停止成功,请确认停止后,等待约15秒后执行返回操作"));
            QTimer::singleShot(15000, [=]() { //大概停止10秒后文件的最后修改时间
                QFile file(lastFilePath);
                if (!file.remove()) {
                    qWarning() << "删除文件失败" << file.errorString() << "-错误代码:" << file.error() << "Windows error:" << GetLastError();
                    LOG_ERROR(tr("扫描仪在结束采集任务时,删除文件%1失败, 请手动删除").arg(lastFilePath));
                } else {
                    LOG_INFO(tr("扫描仪停止采集任务成功,请继续操作"));
                };
                });

        } else {
            qWarning() << "停止扫描失败";
            LOG_ERROR(tr("扫描仪停止失败, 请检查设备状态"));
        }
    } else {
        QString FileNames;
        foreach(const auto& fileInfo, currentFiles) {
            FileNames.append(QString("%1\t%2\n").arg(fileInfo.fileName()).arg(fileInfo.birthTime().toString("hh:mm:ss.zzz")));
        }
        QJsonObject obj;
        obj["FileNames"] = FileNames;
        obj["Time"] = QDateTime::currentDateTime().toMSecsSinceEpoch();
        Session session(sModuleScanner, "onWatcherFilesChanged", QJsonArray{ obj });
        gShare.on_session(session.GetRequest());
    }
    //开启了实时预览
    if (gSettings->value("IsRealtimePreview").toBool()) {
        qsizetype filecount = currentFiles.count();
        if (filecount >= 2) {
            QString filepath = currentFiles[filecount - 2].absoluteFilePath();
            QStringList args = { "-f", filepath };
            QString exePath = gShare.appPath + "/bin/FaroPreview.exe";
            qDebug() << "启动预览:" << exePath << args;
            emit gTaskManager.flush_once();//更新任务里程数据
            gShare.shellProcess(exePath, args);
        }
    }
}

void FaroControl::addPreviewFile(const QString& path) {
    /* 后续优化方向，是保存文件名称，对比后只要新增加的 使用的任务文件刷新一次（20次刷新缓存后不用立马刷新）*/
    QDir dir(path);
    // 只监控 .txt 和 .log 文件
    QStringList filters;
    filters << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG" << "*.bmp" << "*.BMP" << "*.tiff" << "*.TIFF" << "*.tif" << "*.TIF";
    QFileInfoList currentFiles = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot,QDir::Time);// 时间排序
    if (currentFiles.isEmpty()) {
        return;
    }
    QString lastFilePath = currentFiles.first().absoluteFilePath();//最新 last()最旧，即时间最早的，这里需要是最新生成的文件
    
    //测试是否需要等待
    QFileInfo fileInfo(lastFilePath);
    // 检查文件权限
    if (!fileInfo.isReadable()) {
        qDebug() << "文件不可读，等待权限:" << lastFilePath;
        return;
    }
    // 检查文件是否稳定 等待100ms后基本稳定了
    qint64 lastSizes{ -1 };
    qint64 currentSize = 0;
    for (size_t i = 0; i < 10; i++) {
        // 重新刷新文件信息以获取最新状态
        fileInfo.refresh();  // 关键：刷新文件信息缓存
        currentSize = fileInfo.size();
        if (lastSizes == currentSize && currentSize > 0) {
            qDebug() << "文件稳定，大小:" << currentSize;
            break;//认为稳定了
        }
        qDebug() << i << "当前文件大小:" << currentSize << "上次文件大小:" << lastSizes << "等待文件稳定中...";
        lastSizes = currentSize;
        QThread::msleep(20);
    }
    static quint8 invoke_module = share::ModuleName::scanner;
    static QByteArray data;
    //读取文件内容
    QFile file(lastFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "打开文件失败" << file.errorString();
        return;
    }
    data = file.readAll();
    QString fileName = fileInfo.fileName();
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::ReadWrite);
    stream << invoke_module << fileName << data;
    qDebug() << "扫描仪发送图片二进制数据:" << bytes.size() << "字节";
    emit gShare.sigSentBinary(bytes);//推送二进制数据
}

