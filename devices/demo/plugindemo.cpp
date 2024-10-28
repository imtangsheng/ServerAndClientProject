#include "plugindemo.h"

#include <QMetaProperty>

PluginDemo::PluginDemo(QObject *parent)
    : PluginInterfaceDevice(parent)
{

    state = State::Uninitialized;
    // 方法1: 直接从插件元数据中获取

    const QMetaObject* metaObj = this->metaObject();
    // 获取属性数量
    int propertyCount = metaObj->propertyCount();
    qDebug() << "Property count:" << propertyCount;

    // 获取属性名称
    for (int i = 0; i < propertyCount; ++i) {
        QMetaProperty metaProperty = metaObj->property(i);
        qDebug() << "Property name:" << metaProperty.name();
    }
    // Property count: 1
    // Property name: objectName
}

PluginDemo::~PluginDemo()
{
    qDebug()<<"PluginDemo::~PluginDemo()";
    this->destroy();
}

QString PluginDemo::name() const
{
    return "Test";
    // QPluginMetaData()
}

QString PluginDemo::version() const
{
    return "0.0.1";
}

Result PluginDemo::awake()
{
    return 1;
}

Result PluginDemo::start()
{
return 1;
}

void PluginDemo::update()
{

}

Result PluginDemo::stop()
{
return 1;
}

void PluginDemo::destroy()
{

}

void PluginDemo::executeFeature(const QString &featureName)
{

}
