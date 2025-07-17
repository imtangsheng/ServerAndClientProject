/*!
 * @file CameraPlugin.cpp
 * @brief 相机统一的通用接口 SDK 的API开发的实现文件->cn
 * @date 2025-04
 */
#include "CameraPlugin.h"

#ifdef DEVICE_TYPE_CAMERA_HiKvision
#include "MVS/hikvision_camera.h"
static ICameraBase* gCameraSDK = new HiKvisionCamera();
#elif defined(DEVICE_TYPE_CAMERA_DvpLineScan)
#include "LineScanCamera/dvp_line_scan_camera.h"
static ICameraBase* gCameraSDK = new DvpLineScanCamera();
#else
//qFatal(tr("No camera type defined").toUtf8().constData());
#endif

static QString g_plugin_module_name = share::Shared::GetModuleName(share::ModuleName::camera);

CameraPlugin::CameraPlugin() {
//#ifdef CAMERA_TYPE_HiKvision
//    //gCameraSDK = new HiKvisionCamera();
//#elif defined(CAMERA_TYPE_DvpLineScan)
//    //gCameraSDK = new DvpLineScanCamera();
//#else
//    qFatal(tr("No camera type defined").toUtf8().constData());
//    //qCritical() << "Camera initialization failed:" << e.what();
//#endif
    IPluginDevice::initialize();
}

CameraPlugin::~CameraPlugin() {
    qDebug() << "#PluginCamera~CameraPlugin()";
}

QString CameraPlugin::_module() const {
    return g_plugin_module_name;
}
void CameraPlugin::initialize() {
    //此处头文件包含为局部类的实现
    gCameraSDK->initialize();
}

Result CameraPlugin::disconnect() {
    qDebug() << "#PluginCamera断开函数";
    return Result();
}

QString CameraPlugin::name() const {
    return gCameraSDK->DeviceName();
}

QString CameraPlugin::version() const {
    return QString("0.0.1");
}

Result CameraPlugin::OnStarted(CallbackResult callback) {
    return gCameraSDK->OnStarted(callback);
}

Result CameraPlugin::OnStopped(CallbackResult callback) {
    return gCameraSDK->OnStopped(callback);
}

void CameraPlugin::initUi(const Session& session) {
    QJsonObject obj;// = session.params.toObject();
    obj["format"] = gCameraSDK->has_image_format;

    emit gSigSent(session.ResponseString(obj, tr("succeed")), session.socket);

    //onConfigChanged();//界面参数变化主动获取更新
    emit gSigSent(Session::RequestString(2, _module(), "onConfigChanged", QJsonArray{ config_ }), session.socket);
}

void CameraPlugin::SaveConfig(const Session& session) {
    config_ = session.params.toObject();
    Result result = WriteJsonFile(ConfigFilePath(), config_);
    if (!result) {
        LOG_WARNING(result.message);
    }
    gShare.on_send(result, session);
}

void CameraPlugin::execute(const QString& method) {
    qDebug() << "#PluginCamera执行函数" << method;
}

void CameraPlugin::GetImageFormat(const Session& session) {
    //获取照片格式
    gShare.on_session(session.ResponseString(g_image_format), session.socket);
}

void CameraPlugin::SetImageFormat(const Session& session) {
    
    QString format = session.params.toString();
    if (format.isEmpty()) return;
    gCameraSDK->SetImageFormat(format);
    {
        QMutexLocker locker(&_mutex_config);
        QJsonObject general = config_["general"].toObject();
        general["format"] = format;
        config_["general"] = general;
    }
    Result result = WriteJsonFile(ConfigFilePath(), config_);
    gShare.on_send(result, session);
}

void CameraPlugin::scan(const Session& session) {
    Result result = gCameraSDK->scan();
    if (result) {
        emit gSigSent(session.ResponseString(gCameraSDK->GetDeviceIdList(),tr("succeed")));
    } else {
        emit gSigSent(session.ErrorString(result.code, result.message));
    }
}

void CameraPlugin::open(const Session& session) {
    Result result = gCameraSDK->open();
    gShare.on_send(result, session);
}

void CameraPlugin::start(const Session& session) {
    gCameraSDK->start(session);
}

void CameraPlugin::stop(const Session& session) {
    Result result = gCameraSDK->stop();
    gShare.on_send(result, session);
}

void CameraPlugin::trigger(const Session& session) {
    Result result = gCameraSDK->triggerFire();
    gShare.on_send(result, session);
}

void CameraPlugin::onConfigChanged() {
    emit gSigSent(Session::RequestString(2, _module(), "onConfigChanged", QJsonArray{ config_ }));
}

void CameraPlugin::GetUpdateFrameInfo(const Session& session) {
    //Result result = gCameraSDK->slotDispRate();
    Result result(1, "TODO");
    gShare.on_send(result, session);
}

//保存相机参数 有设备参数 和 任务参数
void CameraPlugin::SetCamerasParams(const Session& session) {
    QJsonObject root = session.params.toObject();
    Result result = gCameraSDK->SetCameraConfig(root);
    gShare.on_send(result, session);
}

void CameraPlugin::show(const Session& session) {
    //Result result = gCameraSDK->Property();
    Result result(1, "TODO");
    gShare.on_send(result, session);
}
