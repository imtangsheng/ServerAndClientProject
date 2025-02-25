/*!
 * @file TrolleyController.h
 * @brief 小车控制器 ->cn
 * @date 2025-02-24
 */
#pragma once
#include "iController.h"
class TrolleyController: public ControllerBase
{
	Q_OBJECT
public:
	explicit TrolleyController(QObject* parent = nullptr, QString module = "Trolley");
	~TrolleyController();
	void initialize()  override;

public slots:
	//interface IControllerSDK
	Result  prepare() override;
	Result start() override;
	Result stop() override;
	//Result pause() override;
	Result shutDown() override;
};

inline QPointer<TrolleyController> gTrolleyController;