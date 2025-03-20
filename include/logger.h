/**
 * @file logger.h
 * @brief 日志类头文件
 * @author Tang
 * @date 2025-03-10
 */
#ifndef LOGGER_H
#define LOGGER_H
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QDir>

#define gLog Logger::instance()
#define g_new_message gLog.new_message
// 便捷的宏，用于包含文件、函数、行号信息
#define LOG_DEBUG(msg) gLog.log(msg, LogLevel::Debug,__FUNCTION__, __LINE__, Q_FUNC_INFO)
#define LOG_INFO(msg) gLog.log(msg,LogLevel::Info,__FUNCTION__, __LINE__, Q_FUNC_INFO)
#define LOG_WARNING(msg) gLog.log(msg,LogLevel::Warning,__FUNCTION__, __LINE__, Q_FUNC_INFO)
#define LOG_ERROR(msg) gLog.log(msg,LogLevel::Error, __FUNCTION__, __LINE__, Q_FUNC_INFO)
#define LOG_FATAL(msg) gLog.log(msg, LogLevel::Fatal,__FUNCTION__, __LINE__, Q_FUNC_INFO)

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
	Debug = 0x0001,
	Info = 0x0002,
	Warning = 0x0004,
	Error = 0x0008,
	Fatal = 0x0010,
	All = Debug | Info | Warning | Error | Fatal // 0x001F
};

/**
 * @brief 日志消息结构体
 */
struct LogMessage {
	QString message;       // 日志消息内容
	LogLevel level;        // 日志级别
	QString timestamp;     // 时间戳
	// QString fileName;      // 文件名
	QString className;     // 类名
	QString functionName;  // 函数名
	int lineNumber;        // 行号
};

/**
 * @brief 日志基类
 * 实现日志记录到文件和显示到界面的功能
 */
class SHAREDLIB_EXPORT Logger : public QObject
{
	Q_OBJECT
public:
	QMutex writeMutex;// 多文件时,单例的写锁
	LogLevel logLevel{ LogLevel::All };
	QString logPath{ "" };
	QString baseName{ "log" };
	bool consoleOutput{ false };
	QFile* logFile{ nullptr };
	QDate currentLogDate;
	/**
	 * @brief 构造函数（单例模式）也可以单独模块化使用
	 * @param parent 父对象
	 */
	explicit Logger(QObject* parent = nullptr);

	/**
	 * @brief 获取单例实例
	 * @return Logger实例
	 */
	static Logger& instance() {
		static Logger self;
		return self;
	}

	/**
	* @brief 初始化日志系统
	* @param path 日志文件路径
	* @param name 日志文件名
	* @param level 日志级别
	* @param console 是否在控制台显示日志
	*/
	void init(const QString& path = QString(), const QString& name = QString(), LogLevel level = LogLevel::All, bool console = false);
	// 安装消息处理钩子
	void InstallMessageHandler();
	// 移除消息处理钩子
	void UninstallMessageHandler();
	/**
	 * @brief 写入日志
	 * @param message 日志消息
	 * @param level 日志级别
	 * @param function 函数名
	 * @param line 行号
	 */
	void log(const QString& message, LogLevel level, const char* function, int line, const char* className);


protected:
	/**
	 * @brief 析构函数
	 */
	~Logger();


	QString FormatTheMessage(const QString& message, LogLevel level, const char* function, int line, const char* className);
	// 用于保存之前的消息处理函数,输出重定向
	static QtMessageHandler previousMessageHandler;
	// 自定义消息处理函数
    static void MessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message);
	// 将Qt消息类型转换为日志级别
    static LogLevel QtTypeToLogLevel(QtMsgType type);

    void SetupLogUpdateTimer();// 设置午夜计时器
    void CleanupOldLogFiles() const;//清理旧日志文件

signals:
	/**
	 * @brief 新日志消息信号
	 * @param message 格式化后的日志消息
	 * @param level 日志级别
	 */
    void new_message(const QString& message, LogLevel level = LogLevel::Debug);

private slots:
	/**
	 * @brief 检查日志文件日期
	 */
    void check_log_file_date();

    void log_a_file(const QString& message, LogLevel level);
};


#endif // LOGGER_H
