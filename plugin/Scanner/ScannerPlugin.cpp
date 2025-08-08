/*!
 * @file ScannerPlugin.cpp
 * @brief
 * @date 2025-02
 */
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
    gShare.RegisterHandler(_module(), this);// 设备连接前不加入注册服务中
}

ScannerPlugin::~ScannerPlugin()
{
    qDebug() << "[#Scanner]析构函数";
}

QString ScannerPlugin::_module() const
{
    static QString moduleName = share::Shared::GetModuleName(share::ModuleName::scanner);
    return moduleName;
}

Result ScannerPlugin::activate(QJsonObject param)
{
    QString ip = param.value("ip").toString();
    if (!ip.isEmpty()) {
        gFaroCtrl->ip = ip;
    }
    return initialize();
}

Result ScannerPlugin::initialize()
{
    Result result(false);
    int ret = gFaroCtrl->Connect();
    switch (ret) {
    case static_cast<int>(faro::OK)://0
        qDebug() << "连接成功";
        if (gFaroCtrl->isConnect()) {
            return gShare.RegisterHandler(_module(), this);// 设备连接前不加入注册服务中
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
    qDebug() << "连接结果" << ret;
    if (gFaroCtrl->isConnect()) {
        qDebug() << "连接结果错误" << ret<< "但是状态显示已经连接";
        return gShare.RegisterHandler(_module(), this);// 设备连接前不加入注册服务中
    }
    return result;
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
return Result::Success();
}

Result ScannerPlugin::OnStopped(CallbackResult callback)
{
return Result::Success();
}

void ScannerPlugin::initUi(const Session &session)
{
    QJsonObject obj;
    obj.insert("name", "Faro");
    obj.insert("version", "0.0.1");
    obj.insert("isConnect", gFaroCtrl->isConnect());
    qDebug() << "[时间]" << QDateTime::currentDateTime().toString("hh:mm:ss.zzz") << "[#Faro]连接状态查询" << gFaroCtrl->isConnect();
    gShare.on_session(session.ResponseString(obj),session.socket);
}


void ScannerPlugin::execute(const QString &method)
{
    qDebug() << "[#Scanner]执行方法" << method;
    //if (method == "shutdown") {
    //    gFaroCtrl->shutdown();
    //}
    if(gFaroHandle != nullptr){
        gFaroHandle->deleteLater();
        gFaroHandle = nullptr;
    }
    
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
            gShare.RegisterHandler(_module(), this);// 设备连接前不加入注册服务中
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
    gFaroCtrl->SetParameters(param);
    gShare.on_success("参数设置", session);
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
    QMetaObject::invokeMethod(gFaroHandle, "CreateCameraFocalByScanFile",
        Qt::QueuedConnection,
        Q_ARG(QJsonObject, param));
}
