// imageserver.h
#ifndef IMAGESERVER_H
#define IMAGESERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QDir>
#include <QFileSystemWatcher>

class ImageServer : public QObject
{
    Q_OBJECT
public:
    explicit ImageServer(QObject *parent = nullptr);
    bool startServer(const QString &imagePath, quint16 port = 12345);

private slots:
    void handleNewConnection();
    void handleClientDisconnected();
    void handleClientRequest();
    void handleDirectoryChanged(const QString &path);

private:
    void sendImageList(QTcpSocket *client);
    void sendImage(QTcpSocket *client, const QString &imageName);
    QByteArray createHeader(const QString &command, const QByteArray &data = QByteArray());

    QTcpServer *server;
    QString imageDirPath;
    QFileSystemWatcher *dirWatcher;
    QList<QTcpSocket*> clients;
};

#endif // IMAGESERVER_H
