QtMessageHandler Logger::previousMessageHandler = nullptr;

Logger::Logger(QObject* parent)
	: QObject(parent)
{
}

void Logger::init(const QString& path, const QString& name,
	LogLevel level, bool console) {
	logPath = path.isEmpty() ? QDir::currentPath() : path;
	baseName = name.isEmpty() ? "log" : name;
	logLevel = level;
	consoleOutput = console;
	// 确保日志目录存在
	QDir dir(logPath);
	if (!dir.exists()) { dir.mkpath("."); }
	currentLogDate = QDate::currentDate();
	// 初始创建日志文件
	check_log_file_date();
	// 设置计时器
	SetupLogUpdateTimer();
}

Logger::~Logger()
{
	// 关闭日志文件
	if (logFile) if (logFile->isOpen()) logFile->close();

}

void Logger::log(const QString& message, LogLevel level, const char* function, int line, const char* className)
{
	// 检查日志级别
	if (level < logLevel) {
		return;
	}
	//记录发送到信号
	emit new_message(message, level);
	QString logMessage = FormatTheMessage(message, level, function, line, className);
	log_a_file(logMessage);
	// 同时输出到控制台
#ifdef QT_DEBUG
	 // 输出到控制台 out.setCodec("UTF-8");
	if (consoleOutput) {
		static QTextStream out(stdout);
		out.setEncoding(QStringConverter::System); // 使用系统编码
		// 或者明确指定编码
		// out.setEncoding(QStringConverter::Utf8);
		out<< logMessage << Qt::endl;
	}
#endif
}
QString Logger::FormatTheMessage(const QString& message, LogLevel level, const char* function, int line, const char* className)
{
	//switch (level) { //O(1) - 常数时间，但有多个分支比较
	//case Debug: levelStr = "Debug"; break;
	//case Info: levelStr = "Info"; break;
	//case Warning: levelStr = "Warning"; break;
	//case Error: levelStr = "Error"; break;
	//case Fatal: levelStr = "Fatal"; break;
	//default: levelStr = "Unknown";
	//}
	// 优化方法1：使用静态QMap作为查找表 QMap查找法：O(log n) - n是映射中的元素数量（这里是5）
	// 优化方法2：使用静态QHash作为查找表（更快）QHash查找法：平均O(1)，最差O(n)
	//static const QHash<LogLevel, QString> logLevelHash = {
	//	{ LogLevel::Debug, "DEBUG" },
	//	{ LogLevel::Info, "INFO" },
	//	{ LogLevel::Warning, "WARNING" },
	//	{ LogLevel::Error, "ERROR" },
	//	{ LogLevel::Fatal, "FATAL" }
	//};
	//logLevelHash.value(level, "UNKNOWN");
	// 优化方法3：使用数组查找（最快）数组索引法：严格O(1) - 直接内存访问，没有比较操作
	static const QString logLevelStrings[] = {
		"UNKNOWN", // 0
		"DEBUG",   // 1 (0x0001)
		"INFO",    // 2 (0x0002)
		"UNKNOWN", // 3 (0x0003) - 无效组合
		"WARNING", // 4 (0x0004)
		"UNKNOWN", // 5-15 - 无效组合
		"UNKNOWN",
		"UNKNOWN",
		"ERROR",   // 8 (0x0008)
		"UNKNOWN", // 9-15 - 无效组合
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"UNKNOWN",
		"FATAL"    // 16 (0x0010)
	};
	QString levelStr = logLevelStrings[int(level)];
	QString result = QString("[%1][%2]").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")).arg(levelStr);
	if (className) result += QString("[className#%1]").arg(className);
	if (function) result += QString("[function#%1]").arg(function);
	if (line) result += QString("[line#%1]").arg(line);
	result += QString("%1").arg(message).trimmed();//去掉字符串中的换行符和空格
	return result;
}

void Logger::InstallMessageHandler() {
	previousMessageHandler = qInstallMessageHandler(MessageHandler);
}

void Logger::UninstallMessageHandler() {
	qInstallMessageHandler(previousMessageHandler);
}
void Logger::MessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& message) {
	LogLevel level = QtTypeToLogLevel(type);
	Logger::instance().log(message, level, context.function, context.line, context.category);
	// 如果设置了之前的处理函数，也调用它
	if (previousMessageHandler) {
		previousMessageHandler(type, context, message);
	}
}

LogLevel Logger::QtTypeToLogLevel(QtMsgType type) {
	switch (type) {
	case QtDebugMsg:return LogLevel::Debug;
	case QtInfoMsg:return LogLevel::Info;
	case QtWarningMsg:return LogLevel::Warning;
	case QtCriticalMsg:return LogLevel::Error;
	case QtFatalMsg:return LogLevel::Fatal;
	default:return LogLevel::Debug;
	}
}
#include<QTimer>
void Logger::SetupLogUpdateTimer()
{
	// 计算到今天午夜的时间
	QTime currentTime = QTime::currentTime();
	QTime midnight(0, 0, 0);

	int msecToMidnight;
	if (currentTime < midnight) {
		// 已经过了午夜，计算到下一个午夜的时间
		msecToMidnight = currentTime.msecsTo(midnight);
	}
	else {
		// 计算到下一个午夜的时间
		msecToMidnight = currentTime.msecsTo(QTime(23, 59, 59)) + 1000; // 加1秒到00:00:00
	}

	QTimer::singleShot(msecToMidnight, this, [this]() {
		check_log_file_date();
		SetupLogUpdateTimer(); // 重新设置下一个午夜的计时器
		});
}

void Logger::CleanupOldLogFiles() const
{
	// 限制最大日志文件的数量
	static const int maxFiles = 100;

	QDir dir(logPath);
	QStringList filters;
	filters << QString("%1*.log").arg(baseName);
	QStringList files = dir.entryList(filters, QDir::Files, QDir::Time | QDir::Reversed);

	// 删除多余的日志文件（保留最新的maxFiles个）
	while (files.size() > maxFiles) {
		dir.remove(files.first());
		files.removeFirst();
	}
}
void Logger::check_log_file_date()
{
	QDate newDate = QDate::currentDate();
	// 只有日期变更时才创建新文件
	if (currentLogDate != newDate || !logFile) {
		currentLogDate = newDate;
		QString newLogFileName = QString("%1/%2#%3.log").arg(logPath).arg(baseName).arg(currentLogDate.toString("yyyyMMdd"));

		// 关闭旧文件并创建新文件
		if (logFile) {
			logFile->close();
			delete logFile;
		}

		logFile = new QFile(newLogFileName);
		//使用 QIODevice::Unbuffered 标志打开文件，禁用 Qt 的内部缓冲，直接写入磁盘。
		if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
			qWarning() << "Cannot open log file:" << newLogFileName;
			delete logFile;
			logFile = nullptr;
			return;
		}
		// 清理旧日志文件
		CleanupOldLogFiles();
	}
}

void Logger::log_a_file(const QString& message) {
	QMutexLocker locker(&writeMutex);
	// 写入文件
	if (logFile && logFile->isOpen()) {
		QTextStream stream(logFile);
		stream << message << Qt::endl;
		stream.flush();
	}

}