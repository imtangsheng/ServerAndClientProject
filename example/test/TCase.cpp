#include "TCase.h"

class DeviceProcessV2::Impl {
public:
    Impl() {}
    ~Impl() {}

    bool init() {
        qDebug() << "DeviceProcessV2::Impl::init()";
        return true;
    }
    bool open() {
        qDebug() << "DeviceProcessV2::Impl::open()";
        return true;
    }
    bool close() {
        qDebug() << "DeviceProcessV2::Impl::close()";
        return true;
    }
    bool start() {
        qDebug() << "DeviceProcessV2::Impl::start()";
        return true;
    }
    bool stop() {
        qDebug() << "DeviceProcessV2::Impl::stop()";
        return true;
    }
};

DeviceProcessV2::DeviceProcessV2() : pImpl(new Impl())
{
}

DeviceProcessV2::~DeviceProcessV2() = default;


bool DeviceProcessV2::init() {
    //pImpl = std::make_unique<Impl>();
    return pImpl->init();
}

bool DeviceProcessV2::open() {
    return pImpl->open();
}

bool DeviceProcessV2::close() {
    return pImpl->close();
}

bool DeviceProcessV2::start() {
    return pImpl->start();
}

bool DeviceProcessV2::stop() {
    return pImpl->stop();
}

