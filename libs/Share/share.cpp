#include "share.h"
using namespace south;

Result Share::FindFilePath(const QString& fileName, QString& validConfigPath) {
	// 创建搜索路径列表
	QStringList searchPaths;
	// 应用程序所在目录
	searchPaths << appDirPath + "/" + fileName;  // exe目录下
	// 当前工作目录及其相关路径
	searchPaths << "./" + fileName;              // 当前工作目录
	searchPaths << "../" + fileName;             // 上级目录
	searchPaths << QDir::currentPath() + "/" + fileName;  // 当前路径
	// 搜索文件
	for (const QString& path : searchPaths) {
		if (QFile::exists(path)) {
			validConfigPath = path;
			break;
		}
	}
	// 如果未找到，记录错误信息
	if (validConfigPath.isEmpty()) {
		QString lastError = QString("Config file '%1' not found. Searched paths:").arg(fileName);
		for (const QString& path : searchPaths) {
			lastError += "\n" + path;
		}
		return Result::Failure(lastError);
	}
	return true;
}

Result Share::ReadJsonFile(const QString& filePath, QJsonObject& json) {
	QFile configFile(filePath);
	// 尝试打开文件
	if (!configFile.open(QIODevice::ReadOnly)) {
		return Result::Failure(QString("Failed to open file: %1").arg(filePath));
	}
	// 读取JSON文件内容
	QByteArray data = configFile.readAll();
	configFile.close();
	// 读取并解析JSON数据
	QJsonParseError parseError;
	QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
	if (parseError.error != QJsonParseError::NoError) {
		return Result::Failure(QString("JSON文件%1解析错误:%2").arg(filePath).arg(parseError.errorString()));
	}

	if (jsonDoc.isNull() || !jsonDoc.isObject()) {
		return Result::Failure(QString("文件内容不是有效的对象格式: %1").arg(filePath));
	}
	json = jsonDoc.object();
	return true;
}

Result Share::WriteJsonFile(const QString& filePath, const QJsonObject& json) {
	// 创建JSON文档
	QJsonDocument doc(json);
	QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
	// 写入文件,使用绝对路径,如果目录不存在会无法写入
	QFile file(appDirPath + "/" + filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return Result::Failure("无法打开文件进行写入: " + file.errorString());
	}
	qint64 bytesWritten = file.write(jsonData);
	file.close();
	if (bytesWritten == -1) {
		return Result::Failure("写入文件失败: " + file.errorString());
	}
	return true;
}


void Share::on_send(const Result& result, const Session& session)
{
	if (result)
	{
        emit sent(session.ResponseString(result.message), session.socket);
	}
	else
	{
        emit sent(session.ErrorString(result.code, result.message), session.socket);
	}
}
