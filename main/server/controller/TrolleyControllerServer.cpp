/*!
 * @file TrolleyControllerServer.cpp
 * @brief 小车控制器的实现文件->cn
 * @date 2025-02-24
 */
#include "TrolleyController.h"
TrolleyController::TrolleyController(QObject* parent, QString module):
    ControllerBase(module, parent) {
    initialize();
}

TrolleyController::~TrolleyController()
{
    qDebug() << "TrolleyController::~TrolleyController()";
}

void TrolleyController::initialize()
{
}

Result TrolleyController::prepare()
{
    qDebug() << "TrolleyController::prepare()";
    return Result();
}

Result TrolleyController::start()
{
    qDebug() << "TrolleyController::start()";
    return Result();
}

Result TrolleyController::stop()
{
    qDebug() << "TrolleyController::stop()";
    return Result();
}

#include <QCoreApplication>
Result TrolleyController::shutDown()
{
    qDebug() << "TrolleyController::shutDown()";
    QCoreApplication::exit(0);
    return Result();
}
