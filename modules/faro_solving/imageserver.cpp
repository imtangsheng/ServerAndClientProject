// imageserver.cpp
#include "imageserver.h"
#include <QImageReader>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonArray>

ImageServer::ImageServer(QObject *parent) : QObject(parent)
{
    server = new QTcpServer(this);
    dirWatcher = new QFileSystemWatcher(this);

    connect(server, &QTcpServer::newConnection, this, &ImageServer::handleNewConnection);
    connect(dirWatcher, &QFileSystemWatcher::directoryChanged, this, &ImageServer::handleDirectoryChanged);
}

bool ImageServer::startServer(const QString &imagePath, quint16 port)
{
    imageDirPath = imagePath;
    dirWatcher->addPath(imageDirPath);

    return server->listen(QHostAddress::Any, port);
}

void ImageServer::handleNewConnection()
{
    QTcpSocket *clientSocket = server->nextPendingConnection();

    connect(clientSocket, &QTcpSocket::readyRead, this, &ImageServer::handleClientRequest);
    connect(clientSocket, &QTcpSocket::disconnected, this, &ImageServer::handleClientDisconnected);

    clients.append(clientSocket);
    sendImageList(clientSocket);
}

void ImageServer::handleClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (clientSocket) {
        clients.removeOne(clientSocket);
        clientSocket->deleteLater();
    }
}

void ImageServer::handleClientRequest()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    QByteArray data = clientSocket->readAll();
    QString command = QString::fromUtf8(data.left(4));
    QString payload = QString::fromUtf8(data.mid(4));

    if (command == "LIST") {
        sendImageList(clientSocket);
    } else if (command == "IMAG") {
        sendImage(clientSocket, payload);
    }
}

void ImageServer::handleDirectoryChanged(const QString &path)
{
    // 目录发生变化时，向所有客户端发送更新的图片列表
    for (QTcpSocket *client : clients) {
        sendImageList(client);
    }
}

void ImageServer::sendImageList(QTcpSocket *client)
{
    QDir dir(imageDirPath);
    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp";
    QStringList images = dir.entryList(filters, QDir::Files);

    QJsonArray imageArray;
    for (const QString &image : images) {
        imageArray.append(image);
    }

    QJsonDocument doc(imageArray);
    QByteArray jsonData = doc.toJson();
    QByteArray response = createHeader("LIST", jsonData);

    client->write(response);
}

void ImageServer::sendImage(QTcpSocket *client, const QString &imageName)
{
    QString imagePath = imageDirPath + "/" + imageName;
    QImage image(imagePath);

    if (image.isNull()) return;

    QByteArray imageData;
    QBuffer buffer(&imageData);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "JPG");

    QByteArray response = createHeader("IMAG", imageData);
    client->write(response);
}

QByteArray ImageServer::createHeader(const QString &command, const QByteArray &data)
{
    QByteArray header;
    header.append(command.toUtf8());
    header.append(QString::number(data.size()).rightJustified(10, '0').toUtf8());
    header.append(data);
    return header;
}
