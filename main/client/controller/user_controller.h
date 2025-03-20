/*!
* @file user_controller.h
* @brief 用于声明继承用户的控制类的接口,每个应用程序要自己实现
*/
#pragma once
#include "iController.h"
/*!
 * @class ControllerBase
 * @brief 全局的控制类管理模块 interface IControllerSDK wrapper method implementations
 *
 * @details 这是一个控制类，用于控制继承的调用方法。
 * Qt框架设计上就要求parent参数必须是非const的QObject指针,因此传递的this 是非 const 的 QObject 指针是必要的
 *
 */
class UserController : public QObject,public iController
{
	Q_OBJECT
public:
	explicit UserController(QObject* parent = nullptr,const QString & module = "user");
	~UserController();
    /*iController API 设备控制的接口方法*/
    void initialize() final;
    //interface IControllerSDK wrapper method implementations
    void  prepare() final;
    void start() final;
    void stop() final;

	Q_INVOKABLE void error(const Session& session) {
        emit g_new_message(tr("错误码:%1,%2").arg(session.code).arg(session.message),LogLevel::Error);
        qWarning()<<"发送内容:"<< session.params.toString();
	};
    Q_INVOKABLE void login(const Session& session);
    void acquisition_begins();//发送
    Q_INVOKABLE void acquisition_begins(const Session& session);//接收

protected:
	QString selfName;//外部代码无法直接访问
	QSet<QPointer<QObject>> subscribe;
};

extern QPointer <UserController> gUserController;
