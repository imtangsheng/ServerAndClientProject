#include <QApplication>

#include "ui/WebSocket.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    WebSocket socket;
    socket.show();



    return app.exec();
}
