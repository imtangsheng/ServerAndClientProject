

#include<QTimer>
#include <QtConcurrent>
#include "FaroControl.h"
using namespace faro;
#include "api/faro.ls.sdk.tlh"
// Faro许可证验证
//const BSTR licenseCode = _bstr_t(
//    "FARO Open Runtime License\n"
//    "Key:" FARO_LICENSE_KEY "\n"
//    "\n"
//    "The software is the registered property of "
//    "FARO Scanner Production GmbH, Stuttgart, Germany.\n"
//    "All rights reserved.\n"
//    "This software may only be used with written permission "
//    "of FARO Scanner Production GmbH, Stuttgart, Germany.");

static IiQLicensedInterfaceIfPtr liPtr(nullptr); //许可
static IScanCtrlSDKPtr scanCtrl(nullptr);// = static_cast<IScanCtrlSDKPtr>(liPtr);

#pragma region _IScanCtrlSDKEvents Members

// 事件接收器类
//class CScanEventSink : public _IScanCtrlSDKEvents
//{
//private:
//    LONG m_cRef;
//    
//public:
//    CScanEventSink() : m_cRef(1) {}
//    
//    // IUnknown 接口实现
//    STDMETHODIMP QueryInterface(REFIID riid, void** ppv)
//    {
//        if (riid == IID_IUnknown || riid == IID_IDispatch || 
//            riid == __uuidof(_IScanCtrlSDKEvents))
//        {
//            *ppv = this;
//            AddRef();
//            return S_OK;
//        }
//        *ppv = NULL;
//        return E_NOINTERFACE;
//    }
//    
//    STDMETHODIMP_(ULONG) AddRef()
//    {
//        return InterlockedIncrement(&m_cRef);
//    }
//    
//    STDMETHODIMP_(ULONG) Release()
//    {
//        LONG lRef = InterlockedDecrement(&m_cRef);
//        if (lRef == 0)
//            delete this;
//        return lRef;
//    }
//    
//    // IDispatch 接口实现
//    STDMETHODIMP GetTypeInfoCount(UINT* pctinfo)
//    {
//        *pctinfo = 0;
//        return S_OK;
//    }
//    
//    STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo** ppTInfo)
//    {
//        return E_NOTIMPL;
//    }
//    
//    STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, 
//                              UINT cNames, LCID lcid, DISPID* rgDispId)
//    {
//        return E_NOTIMPL;
//    }
//    
//    // 最重要的方法 - 处理事件调用
//    STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
//                       WORD wFlags, DISPPARAMS* pDispParams, 
//                       VARIANT* pVarResult, EXCEPINFO* pExcepInfo, 
//                       UINT* puArgErr)
//    {
//        switch(dispIdMember)
//        {
//            case 0x1: // scanCompleted 的 DISPID
//                return scanCompleted();
//            default:
//                return DISP_E_MEMBERNOTFOUND;
//        }
//    }
//    
//    // 实现具体的事件处理
//    STDMETHODIMP scanCompleted()
//    {
//        //std::cout << "扫描完成事件被触发!" << std::endl;
//        // 在这里添加你的事件处理逻辑
//        qDebug() << "扫描完成事件被触发!";
//        return S_OK;
//    }
//};
//class CEventConnection
//{
//private:
//    IConnectionPointContainer* m_pContainer;
//    IConnectionPoint* m_pConnectionPoint;
//    CScanEventSink* m_pEventSink;
//    DWORD m_dwCookie;
//
//public:
//    CEventConnection() : m_pContainer(NULL), m_pConnectionPoint(NULL),
//        m_pEventSink(NULL), m_dwCookie(0) {
//    }
//
//    ~CEventConnection() {
//        Disconnect();
//    }
//
//    // 连接到COM对象的事件
//    HRESULT Connect(IUnknown* pSource) {
//        HRESULT hr;
//
//        // 获取连接点容器
//        hr = pSource->QueryInterface(IID_IConnectionPointContainer,
//            (void**)&m_pContainer);
//        if (FAILED(hr))
//            return hr;
//
//        // 查找事件接口的连接点
//        hr = m_pContainer->FindConnectionPoint(__uuidof(_IScanCtrlSDKEvents),
//            &m_pConnectionPoint);
//        if (FAILED(hr)) {
//            m_pContainer->Release();
//            m_pContainer = NULL;
//            return hr;
//        }
//
//        // 创建事件接收器
//        m_pEventSink = new CScanEventSink();
//        if (!m_pEventSink) {
//            Disconnect();
//            return E_OUTOFMEMORY;
//        }
//
//        // 建立连接
//        hr = m_pConnectionPoint->Advise(m_pEventSink, &m_dwCookie);
//        if (FAILED(hr)) {
//            Disconnect();
//            return hr;
//        }
//
//        return S_OK;
//    }
//
//    // 断开连接
//    void Disconnect() {
//        if (m_pConnectionPoint && m_dwCookie != 0) {
//            m_pConnectionPoint->Unadvise(m_dwCookie);
//            m_dwCookie = 0;
//        }
//
//        if (m_pConnectionPoint) {
//            m_pConnectionPoint->Release(); 
//            m_pConnectionPoint = NULL;
//        }
//
//        if (m_pContainer) {
//            m_pContainer->Release();
//            m_pContainer = NULL;
//        }
//
//        if (m_pEventSink) {
//            m_pEventSink->Release();
//            m_pEventSink = NULL;
//        }
//    }
//};
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
        LOG_ERROR(tr("#Faro:法如扫描初始化错误: %1").arg(error));
        errorNumbers = -1;
    }
    //ip = "192.168.43.1";
    ip = "172.17.9.20";
}

FaroControl::~FaroControl() {
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

int FaroControl::Connect() const {
    qDebug() << "[#Faro]尝试连接" << ip <<"[时间]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    scanCtrl->ScannerIP = faro::FaroSring(ip);
    //scanCtrl->clearExceptions();
    int ret = scanCtrl->connect(); //可能的结果:0, 2, 4 ⚠️注意,该函数会冻结程序运行,无线连接21秒,并且在有线网络时候,其返回2,结果不准,在约24秒后通过连接查询会连接上
    qDebug() << "FaroControl::connect 连接结果" << ret;
    return ret;
}

int FaroControl::SetParameters(QJsonObject param) {
    /* ScanMode 可以有四个值：
    0 固定灰色 设置 StationaryGrey 以进行普通球面灰度扫描。
    1 固定颜色 设置 StationaryColor 以记录带颜色的球面扫描
    2 螺旋灰 将 HelicalGrey 设置为使用 TTL 接口以螺旋模式进行扫描。
    3 螺旋CANGrey 将 HelicalCANGrey 设置为使用 CAN 通信以螺旋模式进行扫描。
    */
    QString path = "D:/Test/Data/";
    QDir dir(path);
    // 检查目录是否存在
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {// 递归创建所有需要的目录层级
            LOG_ERROR(tr("[#Faro]目录%1创建失败").arg(path));
            return -1;
        }
    }
    
    scanCtrl->_ScanMode = HelicalCanGrey;
    scanCtrl->_StorageMode = SMRemote;//SMRemote: Store scans on the remote computer (the computer the SDK is running on).
    scanCtrl->ScanFileNumber = 1;
    scanCtrl->ScanBaseName = L"Scan";

    scanCtrl->RemoteScanStoragePath = faro::FaroSring(dir.absolutePath());//(gTaskFileInfo->path + "/" + kTaskDirPointCloudName);

    int resolution = param.value(Json_Resolution).toString("4").toInt();// Only the following values are permitted: 1, 2, 4, 5, 8, 10, 16, 20, 32
    scanCtrl->Resolution = resolution;
    scanCtrl->MeasurementRate = param.value(Json_MeasurementRate).toString("8").toInt();//Possible values: 1, 2, 4, 8. 法如操作界面的质量属性 quality
    scanCtrl->NoiseCompression = 1;//Possible values: 1, 2, 4. 
    //scanCtrl->HorizontalAngleMin = 0;
    //scanCtrl->HorizontalAngleMax = 360;
    scanCtrl->VerticalAngleMin = -60;//In ScanMode HelicalGrey and HelicalCANGrey this parameter will automatically be set to 0° and cannot be changed
    scanCtrl->VerticalAngleMax = 90;

    /*In  ScanMode StationaryGrey NumCols  will  automatically  be  adjusted  every  time,
    when Resolution, HorizontalAngleMin, or HorizontalAngleMax  has  changed.So  this  parameter should be changed at last
    设置要记录的扫描线数。当达到此数字时，扫描仪将停止扫描。也可以选择将扫描数据流自动拆分为多个文件
    */
    scanCtrl->NumCols = param.value(Json_NumCols).toInteger(2000000); // 2000000; //In ScanMode HelicalGrey changes on Resolution or the scan area will have no effect on NumCols.
    scanCtrl->SplitAfterLines = param.value(Json_SplitAfterLines).toInt(5000);//The minimum number is 100. Smaller scan files are not supported.
    int ret = scanCtrl->syncParam();// By calling syncParam they get synchronized with the scanner 
    qDebug() << "执行结果" << ret;
    return ret;
}

int FaroControl::ScanStart() {
    /* After having received the start signal the laser sensor needs approx. 10 scan columns to warm up. In this time the recorded scan data will be unusable
    * 收到启动信号后，激光传感器需要大约 10 个扫描色谱柱以预热。在此期间，记录的扫描数据将无法使用。
    * 请注意，在螺旋扫描模式下，记录的扫描点没有完整的 3D信息，并且必须使用额外的定位信息进行扩展（例如来自里程表或 GPS 的定位信息）。
    */
    // 创建事件处理器并连接
    //static CEventConnection eventConnection;
    //HRESULT hr = eventConnection.Connect(scanCtrl);
    //if (SUCCEEDED(hr)) {
        //qDebug()<< "事件监听已成功建立";

        // 运行消息循环等待事件
        //MSG msg;
        //while (GetMessage(&msg, NULL, 0, 0)) {
        //    TranslateMessage(&msg);
        //    DispatchMessage(&msg);
        //}
    //} else {
        //qDebug() << "建立事件监听失败: " << hr;
    //}

    // 事件连接会在析构函数中自动断开
    //scanCtrl->clearExceptions();//Clears all exceptions on the scanner.This function is automatically called at the beginning of scanStart.
    int ret = scanCtrl->startScan();//0, 1, 3, 4  约等待8秒才可以触发录制的功能,否则点击无效
    qDebug() << "执行结果" << ret;
    //qint32 Errorcode = scanCtrl->NumberExceptions;
    //qDebug() << "##startScan NumberExceptions: " << Errorcode;
    return ret;
}

int FaroControl::ScanRecord() {
    HelicalRecordingStatus status{ HRSUnknown };
    int ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    qDebug() << "FaroControl::ScanRecord 查询结果和状态" << ret << status;
    ret = scanCtrl->recordScan();//0,3 Starts scan data recording when scanning in helical CAN mode.
    qDebug() << "执行开始记录数据结果 " << ret;
    ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    qDebug() << "FaroControl::ScanRecord 再次查询结果和状态" << ret << status;
    return ret;
}

//return: 0, 3 Pauses scan data recording when scanning on helical CAN mode. Restart scan data recording with recordScan.
int FaroControl::ScanPause() {
    //HRSUnknown = 0,
    //HRSPaused = 1,
    //HRSRecording = 2
    HelicalRecordingStatus status{ HRSUnknown };
    int ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    qDebug() << "执行结果" << ret << status;
    qDebug() << "正在暂停";
    ret = scanCtrl->pauseScan();//需要等待生效
    ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    qDebug() << "执行结果" << ret;
    ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    qDebug() << "暂停执行结果查询" << ret << status;
    //if (ret==0) {
    //    switch (status) {
    //        case HRSUnknown:
    //        case HRSPaused:
    //            qDebug() << "结果显示已经暂停,无需操作" << ret << status;
    //            break;
    //    default:
    //        qDebug() << "正在暂停";
    //        ret = scanCtrl->pauseScan();
    //        ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    //        qDebug() << "执行结果" << ret;
    //        ret = scanCtrl->inquireRecordingStatus(&status);//0, 2, 3, 4 
    //        qDebug() << "暂停执行结果查询" << ret << status;
    //    }
    //}
    
    return ret;
}

int FaroControl::ScanStop() {
    int ret = 0;
    int percent = scanCtrl->ScanProgress;
    int currentNumCols = scanCtrl->NumCols * percent / 100;
    qDebug() << "[#Scanner]停止扫描 进度:" << percent << "%" << currentNumCols;
    int stopNumCols = (currentNumCols / scanCtrl->SplitAfterLines + 1)* scanCtrl->SplitAfterLines;
    //scanCtrl->PutNumCols(stopNumCols);//umCols 必须由用户设置，并且在更改分辨率等其他参数时不会自动调整。
    //ret = scanCtrl->syncParam();// By calling syncParam they get synchronized with the scanner 1541 返回值不知道
    qDebug() << "调整线数执行结果" << ret << stopNumCols;
    /*
    通过调整线数不会有效
    */
    
    ret = scanCtrl->stopScan();//0,1, 3
    qDebug() << "执行结果" << ret;
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

void FaroControl::onDirectoryChanged(const QString& path) {
    {
        // 获取当前文件列表
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
    }
}
