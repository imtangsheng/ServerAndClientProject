#include "logger.h"

QScopedPointer<Logger> Logger::instance;
QMutex Logger::mutex;
QtMessageHandler Logger::previousMessageHandler = nullptr;

Logger* Logger::getInstance() {
    if (instance.isNull()) {
        QMutexLocker locker(&mutex);
        if (instance.isNull()) {
            instance.reset(new Logger());
        }
    }
    return instance.data();
}

Logger::Logger() {
}

Logger::~Logger() {
    if (logFile) {
        logFile->close();
        delete logFile;
    }
}

void Logger::init(const QString& path, const QString& name,
    LogLevel level, bool console) {
    logPath = path.isEmpty() ? QDir::currentPath() : path;
    baseName = name.isEmpty() ? "log" : name;
    logLevel = level;
    consoleOutput = console;

    QDir dir(logPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    openLogFile();
}

void Logger::log(LogLevel level, const QString& msg,
    const char* file, int line, const char* function) {
    if (!isLevelEnabled(level)) {
        return;
    }

    QString text = formatMessage(level, msg, file, line, function);

    QMutexLocker locker(&writeMutex);

    // 检查文件大小
    checkFileSize();

    // 写入文件
    if (logFile && logFile->isOpen()) {
        QTextStream stream(logFile);
        stream << text << "\n";
        stream.flush();
    }

    // 输出到控制台
    if (consoleOutput) QTextStream(stdout) << text << "\n";
}

QString Logger::formatMessage(LogLevel level, const QString& msg,
    const char* file, int line, const char* function) const {
    QString levelStr;
    switch (level) {
    case Debug: levelStr = "Debug"; break;
    case Info: levelStr = "Info"; break;
    case Warning: levelStr = "Warning"; break;
    case Error: levelStr = "Error"; break;
    case Fatal: levelStr = "Fatal"; break;
    default: levelStr = "Unknown";
    }

    QString result = QString("[%1][%2]")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"))
        .arg(levelStr);

    result += QString(" %1").arg(msg);

    if (function)
        result += QString("[%1]").arg(function);
    if (file)
        result += QString("[%1:%2]").arg(file).arg(line);

    return result;
}

void Logger::checkFileSize() {
    if (!logFile || !logFile->isOpen())
        return;

    if (logFile->size() >= maxFileSize) {
        logFile->close();

        // 文件重命名
        QDir dir(logPath);
        QStringList filters;
        filters << QString("%1*.log").arg(baseName);
        QStringList files = dir.entryList(filters, QDir::Files, QDir::Time);

        // 删除多余的日志文件
        while (files.size() >= maxFiles) {
            dir.remove(files.last());
            files.removeLast();
        }

        // 重命名现有文件
        for (int i = files.size() - 1; i >= 0; --i) {
            QString oldName = dir.filePath(files[i]);
            QString newName = QString("%1/%2.%3.log")
                .arg(logPath)
                .arg(baseName)
                .arg(i + 1);
            dir.rename(oldName, newName);
        }

        openLogFile();
    }
}

void Logger::openLogFile() {
    if (logFile) {
        delete logFile;
    }

    logFile = new QFile(getFileName());
    logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

QString Logger::getFileName() const {
    return QString("%1/%2.log").arg(logPath).arg(baseName);
}

void Logger::setLogLevel(LogLevel level) {
    logLevel = level;
}

void Logger::setMaxFileSize(qint64 bytes) {
    maxFileSize = bytes;
}

void Logger::setMaxFileCount(int count) {
    maxFiles = count;
}

void Logger::setConsoleOutput(bool enable) {
    consoleOutput = enable;
}

bool Logger::isLevelEnabled(LogLevel level) const {
    return (logLevel & level);
}

void Logger::installMessageHandler() {
    previousMessageHandler = qInstallMessageHandler(messageHandler);
}

void Logger::uninstallMessageHandler() {
    qInstallMessageHandler(previousMessageHandler);
}

void Logger::messageHandler(QtMsgType type,
    const QMessageLogContext& context,
    const QString& message) {
    LogLevel level = qtTypeToLogLevel(type);

    Logger::getInstance()->log(level, message,
        context.file,
        context.line,
        context.function);

    // 如果设置了之前的处理函数，也调用它
    if (previousMessageHandler) {
        previousMessageHandler(type, context, message);
    }
}

LogLevel Logger::qtTypeToLogLevel(QtMsgType type) {
    switch (type) {
    case QtDebugMsg:
        return LogLevel::Debug;
    case QtInfoMsg:
        return LogLevel::Info;
    case QtWarningMsg:
        return LogLevel::Warning;
    case QtCriticalMsg:
        return LogLevel::Error;
    case QtFatalMsg:
        return LogLevel::Fatal;
    default:
        return LogLevel::Debug;
    }
}