#include "plugindemo.h"

#include <QMetaProperty>


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
    if(initialized) return 1;
    initialized = true;
    return initialized;
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
