/*!
 * @file MainControl.h
 * @brief 界面的公有变量,单例变量类的集成 ->cn
 * @date 2025-05-28
 */
#ifndef MAINCONTROL_H
#define MAINCONTROL_H
#include <QSet>

#include <QtWebSockets/QWebSocket>

inline QWidget *MainBackgroundWidget;
// #include "TaskManager.h"
class FileInfoDetails; // Add this line
// 为 QPointer<QWebSocket> 提供 qHash 函数
// Qt 6.x 版本的 qHash 函数
inline size_t qHash(const QPointer<QWebSocket>& ptr, size_t seed = 0) noexcept
{
    return qHash(ptr.data(), seed);
}
#define gControl MainControl::instance()
class  MainControl : public QObject
{
    Q_OBJECT
public:
    static MainControl& instance(){
        static MainControl self;
        return self;
    }

    bool carDirection{true};//1是向前,0是向后


    QSet<QPointer<QWebSocket>> sockets; // 自动去重
    void sendTextMessage(const QString &message);
    Result SendAndWaitResult(Session &session,QString info=QString(), quint8 sTimeout = 30);
    void SetBackgroudAcrylicEffect(QWidget *dialog);
private:
    explicit MainControl(QObject* parent = nullptr);
    QString module_;
signals:
    void onProjectClicked(const FileInfoDetails &project);
    void onParamTemplateClicked(int id);

public slots:
     void onEnableChanged(bool enable=true);
};
#endif // MAINCONTROL_H
