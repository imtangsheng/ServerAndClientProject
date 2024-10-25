#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug()<<"start client";
    return app.exec();
}