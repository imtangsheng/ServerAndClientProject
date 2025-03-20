TrolleyController::TrolleyController(QObject* parent, const QString& module)
     :QObject(parent)
{
    name = module;
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
