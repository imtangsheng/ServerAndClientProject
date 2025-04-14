/*!
 * @file TrolleyController.h
 * @brief 小车控制器 ->cn
 * @date 2025-02-24
 */
#pragma once
#include "iController.h"

class TrolleyController: public QObject,public iController
{
	Q_OBJECT
public:
	explicit TrolleyController(QObject* parent = nullptr);
	~TrolleyController();
	/*iController API 设备控制的接口方法*/
	void initialize() final;
	//interface IControllerSDK wrapper method implementations
	void  prepare() final;
	void start() final;
    void stop() final
    {

    }

public slots:
    //Result shutdown(const Session& session);
};

inline QPointer<TrolleyController> gTrolleyController;



