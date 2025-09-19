/*!
 * @file ScannerPlugin.cpp
 * @brief
 * @date 2025-02
 */
#include<QTimer>
#include "ScannerPlugin.h"
#include "Faro/FaroControl.h"
static FaroControl* gFaroCtrl{nullptr};
#include "Faro/FaroHandle.h"
static FaroHandle* gFaroHandle{ nullptr };

//#pragma execution_character_set("utf-8")
ScannerPlugin::ScannerPlugin()
{
    IPluginDevice::initialize();
    qDebug() << "[#Scanner]构造函数" << QThread::currentThread();
    gFaroCtrl = new FaroControl(this);
    gFaroHandle = new FaroHandle();

    gFaroCtrl->SetScanCompletedCallback([this]() {
        qDebug() << "[#Scanner]扫描完成";
        state_ = StateEvent::Finished;
    });
}

ScannerPlugin::~ScannerPlugin()
{
    qDebug() << "[#Scanner]析构函数";
}

QString ScannerPlugin::GetModuleName() const
{
    static QString moduleName = share::Shared::GetModuleName(share::ModuleName::scanner);
    return moduleName;
}

Result ScannerPlugin::Activate_(QJsonObject param)
{
    //判断是否连接,此处设备只有一个,不用对比ip是否一样,直接判断
    if (gFaroCtrl->isConnect()) {
        state_ = StateEvent::Waiting;
        return true;
    }
    //检测是否更换ip地址了
    QString ip = param.value("ip").toString();
    if (gFaroCtrl->ip != ip && !ip.isEmpty()) {
        gFaroCtrl->ip = ip;
    }
    if (!TryConnect()) {
        CheckConnect();
        return Result::Failure(tr("当前连接查询超时,请耐心等待连接状态变化(约45秒)"));//尝试连接失败,此处考虑更换ip的情况,先返回错误
    }
    return true;
}

Result ScannerPlugin::initialize()
{
    QTimer::singleShot(5000, this, [this]() {
        if (!TryConnect()) {
            CheckConnect();
        }
    });

    return true;//有线连接必失败,所以不记录错误信息
}

Result ScannerPlugin::disconnect()
{
    return Result::Success();
}

QString ScannerPlugin::name() const
{
    return QString("Faro");
}

QString ScannerPlugin::version() const
{
    return QString("0.0.1");
}

Result ScannerPlugin::OnStarted(CallbackResult callback)
{
    return gFaroCtrl->OnStarted(callback);
    //QJsonObject content = gTaskFileInfo->data[JSON_TASK_CONTENT].toObject();
    //Result ret = gFaroCtrl->SetParameters(content);
    //if(ret) gFaroCtrl->OnStarted(callback);
    //else {
    //    callback(ret.code, tr("[#Faro]设置参数失败"));
    //}
    //return ret;
}

Result ScannerPlugin::OnStopped(CallbackResult callback)
{
    return gFaroCtrl->OnStopped(callback);
}

Result ScannerPlugin::Shutdown() {
    return gFaroCtrl->shutdown();
}

//使用有线连接的时候,不会立马返回,而是超时,但其实已经连接
Result ScannerPlugin::TryConnect() {
    qDebug() << "[#Faro]尝试连接";
    Result res(false);
    int ret = gFaroCtrl->Connect();
    switch (ret) {
    case static_cast<int>(faro::OK)://0
        qDebug() << "连接成功";
        if (gFaroCtrl->isConnect()) {
            state_ = StateEvent::Waiting;
            return true;
        } else {
            res.message = tr("[#Faro]连接执行结果成功,但是连接状态显示未连接");
            break;
        }
    case static_cast<int>(faro::TIMEOUT)://2
        res.message = tr("连接超时");
        break;
    case static_cast<int>(faro::FAILED)://4
        res.message = (tr("[#Faro]连接%1失败").arg(gFaroCtrl->ip));
        break;
    default:
        res.message = (tr("[#Faro]连接未知错误码:%1").arg(ret));
        break;
    }
    res.code = ret;
    return res;
}


void ScannerPlugin::CheckConnect() {
    static int check_count = 45;
    qDebug() << "检测设备连接状态:" << check_count;
    if (gFaroCtrl->isConnect()) {
        state_ = StateEvent::Waiting;
        return;
    }
    check_count--;
    if (check_count > 0) {
        QTimer::singleShot(1000,this, &ScannerPlugin::CheckConnect);
    } else {
        check_count = 10;
        LOG_WARNING(tr("[#Faro]连接超时:%1,设备不存在或者请重新连接").arg(gFaroCtrl->ip));
    }
}

void ScannerPlugin::initUi(const Session &session)
{
    QJsonObject obj;
    obj.insert("name", "Faro");
    obj.insert("version", "0.0.1");
    obj.insert("isConnect", gFaroCtrl->isConnect());
    qDebug() << "[时间]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "[#Faro]连接状态查询" << gFaroCtrl->isConnect();
    gShare.on_session(session.ResponseSuccess(obj),session.socket);
}


void ScannerPlugin::execute(const QString &method)
{
    qDebug() << "[#Scanner]执行方法" << method;
}

void ScannerPlugin::ScanConnect(const Session& session) {
    if (gFaroCtrl->isConnect()) {
        qDebug() << "已经连接成功";
        gShare.on_success("已经连接", session);
        return;
    }
    Result result(false);
    gFaroCtrl->ip = session.params.toString();
    int ret = gFaroCtrl->Connect();
    switch (ret) {
    case static_cast<int>(faro::OK)://0
        qDebug() << "连接成功";
        if (gFaroCtrl->isConnect()) {
            gShare.RegisterHandler(GetModuleName(), this);// 设备连接前不加入注册服务中
            result = Result::Success(tr("连接成功"));
            break;
        } else {
            result.message = tr("[#Faro]连接执行结果成功,但是连接状态显示未连接");
            break;
        }
    case static_cast<int>(faro::TIMEOUT)://2
        result.message = tr("连接超时");
        break;
    case static_cast<int>(faro::FAILED)://4
        result.message = (tr("[#Faro]连接%1失败").arg(gFaroCtrl->ip));
        break;
    default:
        result.message = (tr("[#Faro]连接未知错误码%1").arg(ret));
        break;
    }
    gShare.on_send(result, session);
}

void ScannerPlugin::SetParameter(const Session& session) {
    qDebug() << "[#Scanner]";
    QJsonObject param = session.params.toObject();
    gShare.on_session(session.Finished(gFaroCtrl->SetParameters(param),"参数设置"), session.socket);
}

void ScannerPlugin::ScanStart(const Session& session) {
    qDebug() << "[#Scanner]开始扫描";
    gFaroCtrl->ScanStart();
    gShare.on_success("开始扫描", session);
}

void ScannerPlugin::ScanRecord(const Session& session) {
    qDebug() << "[#Scanner]扫描记录";
    gFaroCtrl->ScanRecord();
    gShare.on_success("扫描记录", session);
}

void ScannerPlugin::ScanPause(const Session& session) {
    qDebug() << "[#Scanner]暂停扫描";
    gFaroCtrl->ScanPause();
    gShare.on_success("暂停扫描", session);
}

void ScannerPlugin::ScanStop(const Session& session) {

    int nRet = gFaroCtrl->ScanStop();
    gShare.on_send(nRet, session);
    //gShare.on_success("停止扫描", session);

}

void ScannerPlugin::GetProgressPercent(const Session& session) {
    
    int percent = gFaroCtrl->GetScanPercent();
    Session s = session;
    s.params = percent;
    qDebug() << "[#Scanner]获取进度百分比" << percent;
    gShare.on_success("获取进度百分比", s);
}


void ScannerPlugin::GetCameraPositionDistance(const Session& session) {
    //QString dirpath;
    QJsonObject param = session.params.toObject();

    //if (!param.contains(Json_Resolution)) {
    //    param.insert(Json_Resolution, 1);
    //}
    //if (!param.contains(Json_MeasurementRate)) {
    //    param.insert(Json_MeasurementRate, 8);
    //}
    //if (!param.contains(Json_NumCols)) {
    //    param.insert(Json_NumCols, 100);
    //}
    //int ret = gFaroCtrl->SetParameters(param);
    //qDebug() << "[#Scanner]获取>SetParameters结果" << ret;
    //ret = gFaroCtrl->ScanStart();
    //QThread::msleep(8000);
    //ret = gFaroCtrl->ScanRecord();
    //gFaroHandle->CreateCameraFocalByScanFile(param);
    // 异步调用
    gFaroHandle->session = session;
    QMetaObject::invokeMethod(gFaroHandle, "CreateCameraFocalByScanFile",
        Qt::QueuedConnection,
        Q_ARG(QJsonObject, param));
}
