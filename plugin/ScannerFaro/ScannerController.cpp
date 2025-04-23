#include "ScannerController.h"
ScannerController::ScannerController(QObject* parent, const QString& module)
     :QObject(parent)
{
	module_ = south::ShareLib::GetModuleName(south::ModuleName::scanner);
	gSouth.RegisterHandler(module_, this);
	initialize();
}

ScannerController::~ScannerController()
{
	qDebug() << "ScannerController::~ScannerController()";
}

void ScannerController::initialize()
{
	qDebug() << "#Faro:void ScannerController::initialize()";
}

void ScannerController::prepare() {
}

void ScannerController::start() {
}

void ScannerController::stop() {
}


Result ScannerController::shutdown(const Session& session) {
    qDebug() << session.message;
	return Result();
}


