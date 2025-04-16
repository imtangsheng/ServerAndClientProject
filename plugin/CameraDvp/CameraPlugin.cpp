/*!
 * @file CameraPlugin.cpp
 * @brief 度申相机DVP2(Digital Videl Platform 2) SDK 的API开发的实现文件->cn
 * @date 2025-02
 */
#include "CameraPlugin.h"
CameraPlugin::CameraPlugin() {
    IPluginDevice::initialize();
}

CameraPlugin::~CameraPlugin() {
    qDebug() << "#PluginCamera~CameraPlugin()";
}

QString CameraPlugin::_module() const {
    static QString module_name = south::ShareLib::GetModuleName(south::ModuleName::camera);
    return module_name;
}

void CameraPlugin::initialize() {
    //qDebug() <<"#测试:"<<&gController<<QThread::currentThreadId();
    //gCameraController = new CameraController(this,_module());
    //south::ShareLib::instance().registerHandler("camera", gCameraController.data());
    //bool enable =	south::ShareLib::GetConfigSettings()->value("camera/enable",false).toBool();
    //if (!enable) {
        //south::ShareLib::GetConfigSettings()->setValue("camera/enable", true);
    //}
    gCameraSDK = new CameraSDK(this);
}

Result CameraPlugin::disconnect() {
    qDebug() << "#PluginCamera断开函数";
    return Result();
}

Result CameraPlugin::AcquisitionStart() {
    qDebug() << "#PluginCamera开始采集函数";
    return gCameraSDK->start();
}

Result CameraPlugin::AcquisitionStop() {
    qDebug() << "#PluginCamera停止采集函数";
    return gCameraSDK->stop();
}

QString CameraPlugin::name() const {
    return QString("camera");
}

QString CameraPlugin::version() const {

    return QString("0.0.1");
}

void CameraPlugin::execute(const QString& method) {
    qDebug() << "#PluginCamera执行函数" << method;
}

void CameraPlugin::SetImageFormat(const Session& session) {
    QString format = session.params.toString();
    struCameraInfo::format = format;//静态全局变量,保存到文件中
    QJsonObject general = config_["general"].toObject();
    general["format"] = format;
    config_["general"] = general;
    Result result = gSouth.WriteJsonFile(ConfigFilePath(), config_);
    gSouth.on_send(result, session);
}

void CameraPlugin::scan(const Session& session) {
    Result result = gCameraSDK->scan();
    if (result) {
        QString deviceNames = gCameraSDK->devicesIdList.join(",");
        Session session1 = session;
        session1.result = deviceNames;
        emit gSigSent(session1.ResponseString(tr("succeed")));
    } else {
        emit gSigSent(session.ErrorString(result.code, result.message));
    }
}

void CameraPlugin::open(const Session& session) {
    Result result = gCameraSDK->open();
    gSouth.on_send(result, session);
}

void CameraPlugin::start(const Session& session) {
    Result result = gCameraSDK->start();
    gSouth.on_send(result, session);
}

void CameraPlugin::stop(const Session& session) {
    Result result = gCameraSDK->stop();
    gSouth.on_send(result, session);
}

void CameraPlugin::trigger(const Session& session) {
    Result result = gCameraSDK->triggerFire();
    gSouth.on_send(result, session);
}

void CameraPlugin::onConfigChanged() {
    emit gSigSent(Session::RequestString(2, _module(), "onConfigChanged", QJsonArray{ config_ }));
}

void CameraPlugin::GetUpdateFrameInfo(const Session& session) {
    Result result = gCameraSDK->slotDispRate();
    gSouth.on_send(result, session);
}

//保存相机参数 有设备参数 和 任务参数
void CameraPlugin::SetCamerasParams(const Session& session) {
    QJsonObject root = session.params.toObject();
    Result result = gCameraSDK->SetCameraConfig(root);
    gSouth.on_send(result, session);
}

void CameraPlugin::SaveCamerasParams(const Session& session) {
    config_ = session.params.toObject();
    //   QString filePath = obj["filePath"].toString();
    //   QJsonObject cameraParams = obj["params"].toObject();
       //// 检测文件后缀是否为json
       //if (filePath.isEmpty() || !filePath.endsWith(".json", Qt::CaseInsensitive)) {
       //	filePath = CameraConfigFileName;
       //}
       //Result result = gSouth.WriteJsonFile(filePath, cameraParams);
    Result result = gSouth.WriteJsonFile(ConfigFilePath(), config_);
    if (!result) {
        LOG_WARNING(result.message);
    }
    gSouth.on_send(result, session);
}

void CameraPlugin::show(const Session& session) {
    Result result = gCameraSDK->Property();
    gSouth.on_send(result, session);
}
