TrolleyController::TrolleyController(QObject* parent)
     :QObject(parent)
{
    module_ = south::ShareLib::GetModuleName(south::ModuleName::trolley);
    gSouth.RegisterHandler(module_, this);
	initialize();
}

TrolleyController::~TrolleyController()
{
    qDebug() << "#Trolley:TrolleyController::~TrolleyController()";
}

void TrolleyController::initialize()
{
    qDebug() << "#Trolley:void CameraController::initialize()";
}

void TrolleyController::prepare()
{
    qDebug() << "#Trolley:TrolleyController::prepare()";
}

void TrolleyController::start()
{

}
