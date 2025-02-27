/*!
* @file iController.h
* @brief 用于声明继承控制类的接口
*/

/*!
* @defgroup iController MVC架构的控制模块
* @brief 处理逻辑模块描述
*/

/*!
* @ingroup Controller
* @brief 负责处理逻辑，连接Model和View
*/

#pragma once

#include "South.h"
/*!
 * @class ControllerBase
 * @brief 全局的控制类管理模块 interface IControllerSDK wrapper method implementations
 *
 * @details 这是一个控制类，用于控制继承的调用方法。
 * Qt框架设计上就要求parent参数必须是非const的QObject指针,因此传递的this 是非 const 的 QObject 指针是必要的
 *
 */
class ControllerBase : public QObject
{
	//Q_OBJECT
public:
	explicit ControllerBase(const QString& module = "", QObject* parent = nullptr)
		: QObject(parent), selfName(module) {// 创建工作线程 对象有父对象时不能移动到其他线程。Qt的父子对象必须在同一个线程中。
		// 自动注册到管理器
		qDebug() << "控制类管理模块自动注册到管理器:" << selfName;
		gSouth.registerHandler(selfName,this);
		initialize();
	}
	virtual ~ControllerBase() {
		qDebug() << "virtual ~ControllerBase()";
		if (selfThread != nullptr) {
			if (selfThread->isRunning()) {
				selfThread->quit();
				if (!selfThread->wait(5000)) {
					qDebug() << "Thread quit timeout, forcing termination";
					selfThread->terminate();
					selfThread->wait();
				}
			}
			delete selfThread;
			selfThread = nullptr;

		}
	}
	// 派生类可以重写此方法进行线程特定的初始化
	virtual void initialize() {}
//public slots:
	//interface IControllerSDK wrapper method implementations
	virtual Result  prepare() { return false; }
	virtual Result start() = 0;
	virtual Result stop() = 0;
	virtual Result pause() { return false; }
	virtual Result shutDown() { return false; }
protected:
	QString selfName;//外部代码无法直接访问
	QThread* selfThread{ nullptr };
	QSet<QPointer<QObject>> subscribe;
};
