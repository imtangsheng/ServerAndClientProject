/*!
 * @file FaroController.h
 * @brief 法如扫描仪控制器 ->cn
 * @date 2025-02-24
 */

#pragma once
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QFileSystemWatcher>
#include "../iController.h"
class FaroController: public ControllerBase {
	Q_OBJECT
public:
	FaroController(QObject* parent = nullptr, const QString& module = "Faro");
	~FaroController();
	QTimer* timer{ nullptr };

	void initialize()  override;

public slots:
	//interface IControllerSDK
	Result  prepare() override;
	Result start() override;
	Result stop() override;
	//Result pause() override;
	Result shutDown() override;

private:
};

inline QPointer<FaroController> gFaroController = nullptr;