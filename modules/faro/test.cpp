// Test.cpp
#include <QCoreApplication>
#include "RealtimeSolving.h"

int main(int argc, char* argv[])
{
    QCoreApplication a(argc, argv);
    static QDateTime startTime = QDateTime::currentDateTime();
    RealtimeSolving solviong;
    solviong.test();
    qDebug() << "#test end" << startTime.msecsTo(QDateTime::currentDateTime()) << "ms";
    //return a.exec();
}
