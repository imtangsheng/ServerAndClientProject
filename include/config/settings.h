#pragma once

#include <QSettings>
#include <QSharedMemory>
#include <QSystemSemaphore>

#define _path_config "config/config.ini"

class Config
{
private:
    /* data */
    static Config* m_instance;
    Config(/* args */){
        settings = new QSettings(_path_config, QSettings::IniFormat);
        // 在构造函数中注册退出处理函数，退出处理函数都必须是无参数、无返回值的函数
        // std::atexit(shutdownHandler);
        qAddPostRoutine(shutdownHandler);
    };
    ~Config() {
        delete settings;
    }

    static void shutdownHandler() {
        if(m_instance != nullptr){
            delete Config::instance();
            m_instance = nullptr;
        }
    }
public:
    static Config* instance(){
        
        if(m_instance == nullptr){
            static QSharedMemory sharedMemory(_path_config);
            if(sharedMemory.attach(QSharedMemory::ReadOnly)){
                memcpy(&m_instance, sharedMemory.data(), sizeof(Config*));
            }else{
                if(sharedMemory.create(sizeof(Config*))){
                    std::once_flag flag;
                    std::call_once(flag, [=](){
                        m_instance = new Config();
                        sharedMemory.lock();
                        memcpy(sharedMemory.data(), &m_instance, sizeof(Config*));
                        sharedMemory.unlock();
                    });
                }
            }
        }
        return m_instance;

    };
    QSettings* settings;
};
