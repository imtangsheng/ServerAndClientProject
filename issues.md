# issue

- Qt Creator 有进程但无界面问题

	解决方案：清空该文件夹内所有文件(.ini文件)
	C:\Users\username\AppData\Roaming\QtProject 

- **vs2022 编译资源无法处理中文路径**


## 编译器本身
- error: Debugger encountered an exception: Exception at 0x0, code: 0xc0000005: write access violation at: 0x0, flags=0x0 (first chance)

	[重新构造配置后生成]

## 线程归属不匹配
- ASSERT failure in QCoreApplication::sendEvent: "Cannot send events to objects owned by a different thread. Current thread QThread(0x185cb0f4650). Receiver 'TaskManager(0x7fff099a71d0)' was created in thread QThread(0x185cb0ae510, name = "Qt mainThread")", file C:\Users\qt\work\qt\qtbase\src\corelib\kernel\qcoreapplication.cpp, line 531

    **方案1.移除父对象关系**
	**方案2：确保在主线程创建 方案3：使用moveToThread**

## Debug模式下的网络连接报错

### 现象:
	qt.network.ssl: No functional TLS backend was found
### 解决方案:
1. 检查OpenSSL库文件

	确保目标计算机上有正确的OpenSSL DLL文件：

	- libssl-1_1-x64.dll 和 libcrypto-1_1-x64.dll（对于64位系统）
	- 或者 libssl-1_1.dll 和 libcrypto-1_1.dll（对于32位系统）

2. Qt平台插件问题

	确保远程计算机上有完整的Qt运行时库，特别是：

	- platforms 文件夹及其内容
	- tls 文件夹中的SSL插件

## 持有锁调用 lambda函数中存在有返回值错乱破坏栈帧
### 解决方案:
栈帧隔离: 1)创建新的函数对象副本 2)完全释放当前栈帧的锁 3)在新的调用上下文中执行

## ASSERT failure in QCoreApplication::sendEvent: "Cannot send events to objects owned by a different thread
(静态变量的销毁是由 C++ 运行时控制的，而不是由 Qt 的对象树管理)

方案1：静态局部变量不设置父类
static QTimer timer;  // 不设置父对象
方案2：使用指针类型指定父类
QTimer* timer = new QTimer(this);  // 非静态，有父对象


## 关于Debug模式下使用QJsonObject 的contains方法判断是否存在 key关键字的 断言错误 **ASSERT: "lhs.size() == rhs.size()"**

方案:
```
inline static bool SafeHasKey(const QStringList& list,const QString& key) {
    for (auto& k : list) {
        if (k == key) return true;
    }
    return false;
}
```