using namespace share;

QString Shared::GetVersion() {
    static QString version = VERSION_STR;
    return version;
}

Result Shared::FindFilePath(const QString& fileName, QString& validConfigPath) {
    // 创建搜索路径列表
    QStringList searchPaths;
    // 应用程序所在目录
    searchPaths << appPath + "/" + fileName;  // exe目录下
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

void Shared::on_session(const QString& message, QObject* client) {
    emit sigSent(message, client);
};

void Shared::on_success(const QString& msg, const Session& session) {
    emit sigSent(session.ResponseString(0, msg), session.socket);
}

void Shared::on_send(const Result& result, const Session& session) {
    if (result) {
        on_session(session.ResponseString(result.code, result.message), session.socket);
    } else {
        on_session(session.ErrorString(result.code, result.message), session.socket);
    }
}

/**
* @param Session 消息内容的结构体,接收消息结构体
*/
Result Shared::invoke(const Session& session) {
    // 获取目标模块的对象
    QObject* targetObject = handlers[session.module];
    QByteArray utf8Data = session.method.toUtf8();//toUtf8()返回的临时QByteArray对象在语句执行完毕后被销毁，导致指向无效字符的指针。
    const char* method = utf8Data.constData();//method指针指向utf8Data对象,仍然有效。
    bool success{ false };// 调用方法目标模块的对象返回
    /// 1.使用可变参数模板调用 std::apply(func, std::tuple);需要确定参数个数,std::make_index_sequence<>常量类型故不使用
    /// 2.使用模板递归的方法,需要引用常量类型,不支持动态参数
    if (session.params.isArray()) {// 判断是否为数组
        QJsonArray array = session.params.toArray();
        int paramCount = array.size();
        if (paramCount > 10) { // QMetaObject::invokeMethod 最多支持 10 个参数
            qWarning() << "Too many parameters for invokeMethod!";
            return Result(false, session.ErrorString(-2, tr("最多支持 10 个参数")));
        }
        QGenericArgument* argv = new QGenericArgument[paramCount];  // 动态分配参数数组
        QList<QVariant> tempStorage;  // 用来临时存储数据，避免悬空指针,字符传入失败
        // 遍历请求的参数，并根据类型转换为 QGenericArgument
        for (int i = 0; i < paramCount; ++i) {
            QJsonValue param = array[i];
            if (param.isString()) {
                tempStorage.append(param.toString());  // 确保 QString 存活,不然会被释放,其他类型不会
                //argv[i] = QGenericArgument("QString", reinterpret_cast<const void*>(&tempStorage.last()));
                argv[i] = QGenericArgument("QString", &tempStorage.last());
                //argv[i] = Q_ARG(QString, &tempStorage.last().toString());//不可用
            } else if (param.isBool()) {
                tempStorage.append(param.toBool()); argv[i] = QGenericArgument("bool", reinterpret_cast<const void*>(&tempStorage.last()));
            } else if (param.isDouble()) {
                tempStorage.append(param.toDouble()); argv[i] = QGenericArgument("double", &tempStorage.last());
            } else if (param.isObject()) {
                tempStorage.append(param.toObject()); argv[i] = QGenericArgument("QJsonObject", reinterpret_cast<const void*>(&tempStorage.last()));
            } else {
                delete[] argv;  // 释放内存
                return Result(false, session.ErrorString(-2, tr("发送的参数类型是不支持的参数类型")));
            }
        }
        switch (paramCount) {
        case 0:success = QMetaObject::invokeMethod(targetObject, method); break;
        case 1:success = QMetaObject::invokeMethod(targetObject, method, argv[0]); break;
        case 2:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1]); break;
        case 3:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2]); break;
        case 4:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3]); break;
        case 5:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4]); break;
        case 6:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5]); break;
        case 7:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]); break;
        case 8:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7]); break;
        case 9:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8]); break;
        case 10:success = QMetaObject::invokeMethod(targetObject, method, argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]); break;
        default:
            qFatal() << "Qt is support 10 params,Unsupported number of arguments!";
            break;
        }
        delete[] argv;
    } else {
        // 默认使用请求对象作为参数 isNull()是否无效或者未定义值 其他参数调用默认 备注:空的""为字符串类型,会判断为有效值
        success = QMetaObject::invokeMethod(targetObject, method, Q_ARG(const Session&, session));
    }
    if (!success)//如果未找到调用的方法,返回失败,支持函数重载
    {
        return Result(false, session.ErrorString(-3, tr("方法调用失败,请检测模块%1的方法:%2使用的参数是否正确").arg(session.module).arg(session.method)));
    }
    return true;
}

/** 调用异步函数
// 调用异步方法
QFuture<QJsonObject> future = invokeHandlerAsync("module1", "method1", data);
// 方式1: 使用 then() 处理结果
future.then([](const QJsonObject& result) {
    // 处理成功的结果
}).onFailed([](const QException& error) {
    // 处理错误
});
// 方式2: 等待结果
try {
    QJsonObject result = future.result();
    // 处理结果
} catch (const QException& e) {
    // 处理错误
}
**/
QFuture<Result> Shared::invoke_async(const Session& session) {
    // 1. 创建Promise和Future
    // 使用 std::make_shared 来存储 promise
    // 使用 QSharedPointer 创建 promise
    auto promise = QSharedPointer<QPromise<Result>>(new QPromise<Result>());
    QFuture<Result> future = promise->future();
    // 2. 使用QMetaObject::invokeMethod在事件循环中执行操作
    QMetaObject::invokeMethod(this, [this, session, promise]() mutable {
        try {
            // 3. 执行实际操作
            Result success = invoke(session);
            // 4. 设置结果
            if (success) {
                promise->addResult(Result(true, "成功"));// 成功时返回结果
            } else {
                emit sigSent(session.ErrorString(-2, tr("调用失败")), session.socket);
                qWarning() << "Failed to invoke handler" << session.module;
                promise->addResult(Result(false, "调用失败"));// 成功时返回结果
            }
        } catch (const std::exception& e) {
            emit sigSent(session.ErrorString(-2, e.what()), session.socket);
            qWarning() << "Failed to invoke handler" << e.what();
            promise->addResult(Result(false, e.what()));// 成功时返回结果
        }
        // 完成 promise
        promise->finish();
        }, Qt::QueuedConnection);// 指定使用队列连接方式
    // 5. 立即返回future对象
    return future;
}