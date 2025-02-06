#ifndef LOGGER_H
#define LOGGER_H
#pragma execution_character_set("utf-8")
#include <QObject>
#include <QFile>
#include <QDir>
#include <QString>
#include <QDateTime>
#include <QTextStream>
#include <QMutex>
// ��־����ö��
enum LogLevel {
    Debug = 0x0001,
    Info = 0x0002,
    Warning = 0x0004,
    Error = 0x0008,
    Fatal = 0x0010,
    All = Debug | Info | Warning | Error | Fatal
};

class Logger {
public:
    static Logger* getInstance();
    // ��ʼ����־ϵͳ
    void init(const QString& path = QString(),
        const QString& name = QString(),
        LogLevel level = All,
        bool console = false);

    // д��־�ӿ�             
    void log(LogLevel level, const QString& msg,
        const char* file = nullptr,
        int line = 0,
        const char* function = nullptr);

    // ����/��ȡ����
    void setLogLevel(LogLevel level);
    void setMaxFileSize(qint64 bytes);
    void setMaxFileCount(int count);
    void setConsoleOutput(bool enable);

    bool isLevelEnabled(LogLevel level) const;

    // ��װ��Ϣ������
    void installMessageHandler();
    // �Ƴ���Ϣ������
    void uninstallMessageHandler();
    ~Logger();
private:
    Logger();


    QString formatMessage(LogLevel level,
        const QString& msg,
        const char* file,
        int line,
        const char* function) const;

    void checkFileSize();
    void openLogFile();
    QString getFileName() const;

    // ���ڱ���֮ǰ����Ϣ������,����ض���
    static QtMessageHandler previousMessageHandler;
    // �Զ�����Ϣ������
    static void messageHandler(QtMsgType type,
        const QMessageLogContext& context,
        const QString& message);
    // ��Qt��Ϣ����ת��Ϊ��־����                         
    static LogLevel qtTypeToLogLevel(QtMsgType type);

private:
    static QScopedPointer<Logger> instance;
    static QMutex mutex;

    QString logPath;
    QString baseName;
    LogLevel logLevel{All};
    bool consoleOutput{false};

    QFile* logFile{nullptr};
    qint64 maxFileSize{ 10 * 1024 * 1024 };// Ĭ��10MB
    int maxFiles{100};

    QMutex writeMutex;
};

// ��־��
#ifdef QT_DEBUG
#define LOG_DEBUG(msg) Logger::getInstance()->log(Debug, msg, __FILE__, __LINE__, Q_FUNC_INFO)
#else
#define LOG_DEBUG(msg)
#endif

#define LOG_INFO(msg) Logger::getInstance()->log(Info, msg, __FILE__, __LINE__, Q_FUNC_INFO)
#define LOG_WARN(msg) Logger::getInstance()->log(Warning, msg, __FILE__, __LINE__, Q_FUNC_INFO)
#define LOG_ERROR(msg) Logger::getInstance()->log(Error, msg, __FILE__, __LINE__, Q_FUNC_INFO)
#define LOG_FATAL(msg) Logger::getInstance()->log(Fatal, msg, __FILE__, __LINE__, Q_FUNC_INFO)

#endif // LOGGER_H