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
/*!
 * @class iController
 * @brief 全局的控制类管理模块 interface IControllerSDK wrapper method implementations
 *
 * @details 这是一个控制类，用于控制继承的调用方法。
 * Qt框架设计上不支持多继承QObject,因此不继承QObject,其信号与槽由派生类自己实现
 *
 */
#include "global.h"

class iController
{
public:
    //virtual ~iController() {}
    // QSet<QPointer<QObject>> subscribe;
	// 派生类可以重写此方法进行线程特定的初始化
    virtual void initialize()=0;
	//interface IControllerSDK wrapper method implementations
    virtual void  prepare() =0;
    virtual void start() = 0;
    virtual void stop() = 0;
};
