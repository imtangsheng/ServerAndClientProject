/*!
 * @file core_control.h
 * @brief 界面的公有变量,单例变量类的集成 ->cn
 * @date 2025-05-28
 */
#ifndef CORE_CONTROL_H
#define CORE_CONTROL_H

#include <QtWebSockets/QWebSocket>
#include <QSet>

inline QWidget *MainBackgroundWidget;
// #include "TaskManager.h"
class FileInfoDetails; // Add this line
// 为 QPointer<QWebSocket> 提供 qHash 函数
// Qt 6.x 版本的 qHash 函数
inline size_t qHash(const QPointer<QWebSocket>& ptr, size_t seed = 0) noexcept
{
    return qHash(ptr.data(), seed);
}
#define gControl CoreControl::instance()
class  CoreControl : public QObject
{
    Q_OBJECT
public:
    static CoreControl& instance(){
        static CoreControl self;
        return self;
    }

    QSet<QPointer<QWebSocket>> sockets; // 自动去重
    void sendTextMessage(const QString &message);
    Result SendAndWaitResult(Session &session, quint8 sTimeout = 30);
    void SetBackgroudAcrylicEffect(QDialog *dialog);
private:
    CoreControl() = default;

signals:
    void onProjectClicked(const FileInfoDetails &project);

};
#endif // CORE_CONTROL_H
