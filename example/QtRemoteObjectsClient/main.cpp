#include <QCoreApplication>
#include <QtRemoteObjects>
#include <QDebug>
#include "websocketiodevice.h"

/*!
 * @class Client
 * @brief 用于连接信号与槽的类
 *
 * @details 信号与槽不支持函数表达式,所以需要定义一个类来连接信号与槽
 */
class Client : public QObject
{
    Q_OBJECT
public:
    ~Client(){
        qDebug()<<"~Client";
    }
public slots:
    void onMessageReceived(const QString &msg) {
        qDebug() << "Received message:" << msg;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Client client;
    // 创建 WebSocket 客户端
    QScopedPointer<QWebSocket> webSocket{new QWebSocket};

// 连接 WebSocket 服务器
    QObject::connect(webSocket.data(), &QWebSocket::connected, [&]() {
        qDebug() << "WebSocket connected to server.";
    });
    WebSocketIoDevice socket(webSocket.data());
    // 创建 QRemoteObjectNode 并连接到服务器
    QRemoteObjectNode node;
    qDebug() <<"wait connect to node:";
    node.addClientSideConnection(&socket);
    node.setHeartbeatInterval(1000);

    // webSocket->open(QStringLiteral("ws://localhost:12345"));
    webSocket->open(QUrl("ws://localhost:12345"));
    // node.connectToNode(QUrl(QStringLiteral("local:replica"))); // connect with remote host node
    // node.connectToNode(QUrl("tcp://127.0.0.1:12345"));
    // node.connectToNode(QUrl("ws://127.0.0.1:12345"));
    // node.connectToNode(QUrl(QStringLiteral("ws://localhost:12345"))); // 连接到 WebSocket 服务端
    qDebug() <<"wait connect to node:done";
    // 动态获取远程对象
    QSharedPointer<QRemoteObjectDynamicReplica> remoteObject(node.acquireDynamic("MyRemoteObject"));
    qDebug() << "Get to acquire remote object."<<remoteObject.data();
    // 等待远程对象初始化完成
    QObject::connect(remoteObject.data(), &QRemoteObjectDynamicReplica::initialized, [&]() {
        qDebug() << "Client: Remote object initialized";
    });
    if (remoteObject.data()) {
        // 等待远程对象初始化完成
        qDebug() << "等待远程对象初始化完成";
        if (remoteObject->waitForSource()) {
            QString retVal;
            // 设计中信号没有返回值,类似信号返回值都是void
            bool s= remoteObject->metaObject()->invokeMethod(remoteObject.data(),
                                                              "sayHello", Qt::DirectConnection,
                                                              Q_ARG(QString, "World"));
            int i = 11;
            bool s2 = remoteObject->metaObject()->invokeMethod(remoteObject.data(),
                                                              "set", Qt::DirectConnection,
                                                              Q_ARG(int, i));

            // 使用字符串形式连接信号与槽 不支持函数表达式
            QObject::connect(remoteObject.data(), SIGNAL(send1(QString)),
                             &client, SLOT(onMessageReceived(QString)));

            // QObject::connect(remoteObject.data(), SIGNAL(currStateChanged(bool)), this,
            //                  SLOT(recSwitchState_slot(bool)));
            // 调用远程对象的方法
            // QRemoteObjectPendingCall call = remoteObject->callMethod("sayHello", Q_ARG(QString, "World"));
            qDebug() << "hello "<<s<<retVal;
            qDebug() <<"set" <<s2<<i;
        }else{
            qDebug() << "等待远程对象初始化完成12,连接失败网络";
        }
    } else {
        qDebug() << "Failed to acquire remote object.";
    }
    qDebug() << "Failed to acquire remote object.quit";
    return a.exec();
}
#include "main.moc"
