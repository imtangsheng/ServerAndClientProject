#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug()<<"start sever";
    return app.exec();
}