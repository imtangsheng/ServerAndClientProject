#include <QApplication>

#include "ui/WebSocket.h"

#include "ui/ImageViewer.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // WebSocket socket;
    // socket.show();

    ImageViewer viewer;
    viewer.show();

    return app.exec();
}
