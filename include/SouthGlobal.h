#ifndef SOUTHGLOBAL_H
#define SOUTHGLOBAL_H
/**
 * @brief 该文件是通用的数据接口接口类模板，声明全局变量
 *
 * @details
 * Author: Tang
 * Date: 2025-02
 * Version: 0.0.1
*/
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
inline static QJsonObject stringToJson(const QString& jsonString) {
	QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8());
	if (!jsonDoc.isNull() && jsonDoc.isObject()) {
		return jsonDoc.object();
	}
	else {
		return QJsonObject();
	}
}

// 内联函数，将JSON对象转换为字符串
inline static QString jsonToString(const QJsonObject& jsonObject) {
	QJsonDocument jsonDoc(jsonObject);
	return QString::fromUtf8(jsonDoc.toJson());
}

/**定义一个结构体来包含更详细的结果信息**/
struct Result
{
    bool success{ true };
    QString message{""};
    Result(bool s = true, const QString& msg = "") :success(s), message(msg) {}
    operator bool() const { return success; }//重载了 bool 操作符，使其可以像之前的 bool 返回值一样使用例如：if (result)
};
// 声明结构体为元类型
//Q_DECLARE_METATYPE(Result)
// 注册 结构体为元类型 运行时环境初始化时调用  main 函数或者构造函数中
//qRegisterMetaType<Result>("Result");

/**定义一个原子类的结构体**/
#include <QAtomicInteger>
template<typename T>
struct Atomic
{
    QAtomicInteger<T> value;
    QString message;
    // Relaxed - 最宽松的内存序
    // Release - 确保之前的写入对其他线程可见
    // Acquire - 确保之后的读取能看到其他线程的Release写入
    // Ordered - 完全内存屏障
    T get() const { return value.loadAcquire(); }
    void set(T s) { value.storeRelease(s); }
    //在构造函数中，可以直接用 T 类型的值初始化它
    Atomic(T t = T(), const QString& msg = QString()) : value(t), message(msg) {}
    // 重载赋值运算符，允许直接赋值
    Atomic& operator=(T value) {
        set(value);
        return *this;
    }
    // 重载类型转换运算符，允许隐式转换为 T 类型
    operator T() const { return get(); }
};
#include <QAtomicPointer>
template<typename T>
struct AtomicPtr
{
    QAtomicPointer<T> value;
    AtomicPtr(T t = nullptr) : value(t) {}
    T get() const { return value.load(); }
    void set(T s) { value.storeRelease(s); }
    AtomicPtr& operator=(T value) {
        set(value);
        return *this;
    }
    operator T() const { return get(); }
};


#endif // SOUTHGLOBAL_H
