/*!
 * @file FaroController.h
 * @brief 法如扫描仪控制器 ->cn
 * @date 2025-02-24
 */

#pragma once

#include "iController.h"

class ScannerController:public QObject,public iController
{
	Q_OBJECT
public:
	explicit ScannerController(QObject* parent = nullptr);
	~ScannerController();

	/*iController API 设备控制的接口方法*/
	void initialize() final;
	//interface IControllerSDK wrapper method implementations
	void  prepare() final;
	void start() final;
	void stop() final;

	QTimer* timer{ nullptr };

public slots:
    Result shutdown(const Session& session);

private:
};

inline QPointer<ScannerController> gScannerController = nullptr;
