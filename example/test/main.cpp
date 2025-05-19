#include <QCoreApplication>
#include "TCase.h"
import FileReadAndSave;
int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);
    TCase test;
    test.init();
    test.InitImpl();

    DeviceProcess<TCase> * test2 = new TCase();
    test2->init();

    DeviceProcessV2 testV2{};
    testV2.init();

    qDebug() << "Version: " << gSouth.GetVersion();
    TestIxx();
    
    return a.exec();
}
