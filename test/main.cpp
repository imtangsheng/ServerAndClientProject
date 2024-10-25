#include <QCoreApplication>

#include <QDebug>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug()<<"start test";
    std::cout << "hello world" << std::endl;
    return app.exec();
}