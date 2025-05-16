using namespace south;

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

void Shared::on_send(const Result& result, const Session& session) {
    if (result) {
        emit sigSent(session.ResponseString(result.code, result.message), session.socket);
    } else {
        emit sigSent(session.ErrorString(result.code, result.message), session.socket);
    }
}
