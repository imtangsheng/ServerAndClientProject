#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QLockFile>

inline static Result ReadJsonFile(const QString& filePath, QJsonObject& json) {
    QFile configFile(filePath);
    // ���Դ��ļ�
    if (!configFile.open(QIODevice::ReadOnly)) {
        return Result::Failure(QObject::tr("Failed to open file: %1").arg(filePath));
    }
    // ��ȡJSON�ļ�����
    QByteArray data = configFile.readAll();
    configFile.close();
    // ��ȡ������JSON����
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return Result::Failure(QObject::tr("JSON file parse error:%2").arg(filePath).arg(parseError.errorString()));
    }

    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        return Result::Failure(QObject::tr("file is not a valid JSON object: %1").arg(filePath));
    }
    json = jsonDoc.object();
    return true;
}

inline static Result WriteJsonFile(const QString& filePath, const QJsonObject& json) {
    // ��鲢����Ŀ¼
    QFileInfo fileInfo(filePath);
    QString dirPath = fileInfo.path();
    QDir dir(dirPath);
    if (!dir.exists()) {
        if (!dir.mkpath(dirPath)) {
            return Result::Failure(QObject::tr("Failed to create directory: %1").arg(dirPath));
        }
    }
    // ����JSON�ĵ�
    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    // д���ļ���ʹ��QLockFileȷ���ļ�д��Ļ����ԣ�
    QLockFile lockFile(filePath + ".lock");
    if (!lockFile.lock()) {
        return Result::Failure(QObject::tr("Failed to lock file: %1").arg(filePath + ".lock"));
    }
    // д���ļ�,ʹ�þ���·��,���Ŀ¼�����ڻ��޷�д��
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        lockFile.unlock();
        QString errorMsg = QObject::tr("Failed to open file: %1 error:%2").arg(filePath).arg(file.errorString());
        file.close();
        return Result::Failure(errorMsg);
    }
    qint64 bytesWritten = file.write(jsonData);
    file.close();
    lockFile.unlock();
    if (bytesWritten == -1) {
        QString errorMsg = QObject::tr("Failed to write file: %1 error:%2").arg(filePath).arg(file.errorString());
        return Result::Failure(errorMsg);
    }
    return true;
}

// 6.9 �汾���ļ��ж�ȡjsonֵ
inline static Result GetJsonValue(const QString& path, QJsonValue& value) {
    QFile file(path);// 1. �� JSON �ļ�
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Result::Failure(QObject::tr("Failed to open file:%1 Error is:%2").arg(path, file.errorString()));
    }
    QByteArray jsonData = file.readAll();// 2. ��ȡ�ļ�����
    file.close();
    QJsonParseError parseError;// 3. ʹ�� QJsonValue::fromJson ����
    value = QJsonValue::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError) {// 4. ����������
        return Result::Failure(QObject::tr("JSON parsing failed:%1 Error is:%2 At offset:%3").arg(path, parseError.errorString()).arg(parseError.offset));
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
        return Result::Failure(QObject::tr("JSON parsing is %1 Not Object:%2").arg(value.type(), QJsonValue::Object));
    }
}