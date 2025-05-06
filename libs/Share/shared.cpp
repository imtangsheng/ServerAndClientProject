using namespace south;
#include <QLockFile>

Result Shared::FindFilePath(const QString& fileName, QString& validConfigPath) {
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

Result Shared::ReadJsonFile(const QString& filePath, QJsonObject& json) {
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

Result Shared::WriteJsonFile(const QString& filePath, const QJsonObject& json) {
    QString fullPath = appDirPath + "/" + filePath;
    // 检查并创建目录
    QFileInfo fileInfo(fullPath);
    QString dirPath = fileInfo.path();
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(dirPath)) {
            LOG_WARNING("Failed to create directory: " + dirPath);
            return Result::Failure("无法创建目录: " + dirPath);
        }
    }
    // 创建JSON文档
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    // 写入文件（使用QLockFile确保文件写入的互斥性）
    QLockFile lockFile(fullPath + ".lock");
    if (!lockFile.lock()) {
        LOG_WARNING("File is locked by another process");
        return Result::Failure("文件被其他进程锁定");
    }
    // 写入文件,使用绝对路径,如果目录不存在会无法写入
    QFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        lockFile.unlock();
        LOG_WARNING("Failed to Open file: " + file.errorString());
        return Result::Failure("无法打开文件进行写入: " + file.errorString());
    }
    qint64 bytesWritten = file.write(jsonData);
    file.close();
    lockFile.unlock();
    if (bytesWritten == -1) {
        LOG_WARNING("Failed to Write file: " + file.errorString());
        return Result::Failure("写入文件失败: " + file.errorString());
    }
    return true;
}


void Shared::on_send(const Result& result, const Session& session) {
    if (result) {
        emit sigSent(session.ResponseString(result.code, result.message), session.socket);
    } else {
        emit sigSent(session.ErrorString(result.code, result.message), session.socket);
    }
}
