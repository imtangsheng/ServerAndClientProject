#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QLockFile>

inline static Result ReadJsonFile(const QString& filePath, QJsonObject& json) {
    QFile configFile(filePath);
    // 尝试打开文件
    if (!configFile.open(QIODevice::ReadOnly)) {
        return Result::Failure(QObject::tr("打开文件'%1'失败").arg(filePath));
    }
    // 读取JSON文件内容
    QByteArray data = configFile.readAll();
    configFile.close();
    // 读取并解析JSON数据
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return Result::Failure(QObject::tr("文件'%1'解析错误:%2").arg(filePath).arg(parseError.errorString()));
    }

    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        return Result::Failure(QObject::tr("解析的文件不是目标 object类型,文件路径: %1").arg(filePath));
    }
    json = jsonDoc.object();
    return true;
}

inline static Result WriteJsonFile(const QString& filePath, const QJsonObject& json) {
    // 检查并创建目录
    QFileInfo fileInfo(filePath);
    QString dirPath = fileInfo.absolutePath();
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {  // 创建目录（包括所有必要的父目录）
            return Result::Failure(QObject::tr("创建文件'%1'失败").arg(dirPath));
        }
    }
    // 创建JSON文档
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    // 写入文件（使用QLockFile确保文件写入的互斥性）
    QLockFile lockFile(filePath + ".lock");
    if (!lockFile.lock()) {
        return Result::Failure(QObject::tr("文件加锁'%1'失败,可能被占用").arg(filePath + ".lock"));
    }
    // 写入文件,使用绝对路径,如果目录不存在会无法写入
    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        lockFile.unlock();
        QString errorMsg = QObject::tr("文件'%1'打开失败,错误信息:%2").arg(filePath).arg(file.errorString());
        file.close();
        return Result::Failure(errorMsg);
    }
    qint64 bytesWritten = file.write(jsonData);
    file.close();
    lockFile.unlock();
    if (bytesWritten == -1) {
        QString errorMsg = QObject::tr("文件'%1'写入失败,错误信息:%2").arg(filePath).arg(file.errorString());
        return Result::Failure(errorMsg);
    }
    return true;
}

// 6.9 版本的文件中读取json值
inline static Result GetJsonValue(const QString& path, QJsonValue& value) {
    QFile file(path);// 1. 打开 JSON 文件
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Result::Failure(QObject::tr("文件'%1'打开失败,错误信息:%2").arg(path, file.errorString()));
    }
    QByteArray jsonData = file.readAll();// 2. 读取文件内容
    file.close();
    QJsonParseError parseError;// 3. 使用 QJsonValue::fromJson 解析
    value = QJsonValue::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {// 4. 检查解析错误
        return Result::Failure(QObject::tr("文件'%1'解析失败,错误信息:%2 位置:%3").arg(path, parseError.errorString()).arg(parseError.offset));
    }
    return true;
}

inline static Result GetJsonValue(const QString& path, QJsonObject& obj) {
    QJsonValue value;
    Result result = GetJsonValue(path, value);
    if (!result) return result;
    if (value.isObject()) {
        obj = value.toObject();
        return true;
    } else {
        return Result::Failure(QObject::tr("读取的类型是'%1',不是目标类型:%2").arg(value.type(), QJsonValue::Object));
    }
}