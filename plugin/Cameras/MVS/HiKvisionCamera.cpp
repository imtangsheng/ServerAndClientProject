#include "HiKvisionCamera.h"

HiKvisionCamera::HiKvisionCamera() {
    qDebug() << "#PluginCameraHiKvision()";
}

HiKvisionCamera::~HiKvisionCamera() {
    qDebug() << "#PluginCamera~HiKvisionCamera()";
}

bool HiKvisionCamera::initialize() {
    return false;
}

Result HiKvisionCamera::SetCameraConfig(const QJsonObject& config) {
    return Result();
}

Result HiKvisionCamera::scan() {
    return Result();
}

Result HiKvisionCamera::open() {
    return Result();
}

Result HiKvisionCamera::close() {
    return Result();
}

Result HiKvisionCamera::start() {
    return Result();
}

Result HiKvisionCamera::stop() {
    return Result();
}

Result HiKvisionCamera::triggerFire() {
    return Result();
}
