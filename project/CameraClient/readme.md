# Supported Qt Modules
Qt for WebAssembly supports a subset of the Qt modules and features. Tested modules are listed below, other modules may or may not work.
> [this link to the Qt for WebAssembly supports](https://doc.qt.io/qt-6/wasm.html)

    Qt Core
    Qt GUI
    Qt Network
    Qt Widgets
    Qt Qml
    Qt Quick
    Qt Quick Controls
    Qt Quick Layouts
    Qt 5 Core Compatibility APIs
    Qt Image Formats
    Qt OpenGL
    Qt SVG
    Qt WebSockets

---
# 在 Windows 10 系统下安装 Emscripten 的具体步骤
---------------------------------------
1. 首先打开命令行(CMD 或 PowerShell)，查看可用版本：
```
# 克隆仓库
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# 列出所有可用版本
emsdk list
```
---------------------------------------
2. 选择并安装特定版本(以qt6.9.0 版本3.1.70 为例)：
```
# 安装指定版本
emsdk install 3.1.70

# 激活该版本 并 配置环境变量 (也可切换版本)
emsdk activate 3.1.70 --permanent

# 删除版本 3.1.70
emsdk uninstall 3.1.70
# 删除所有未使用的版本
emsdk cleanup
# 更新 SDK 工具本身
emsdk update
```

Web编程
======================================
## **Lambda 表达式中，[&]、[=] 和 [this] 的捕获方式**

### **总结表**
|捕获方式	|捕获内容	|安全性	|适用场景	|Qt 推荐用法|
|----|----|---|----|----|
|`[&]`|     所有外部变量的引用|低|同步执行、短生命周期操作    |避免在异步场景使用|
|`[=]`|     所有外部变量的副本|高|异步信号槽、定时器、网络请求  |配合上下文对象（如 this）|
|`[this]`|  当前对象指针     |中|需要访问类成员           |确保对象生命周期|

**黄金法则**：
在 Qt 的异步上下文中（信号槽、定时器、网络回调），**优先使用 `[=]` 或显式值捕获**，并传递上下文对象（如 `this`）以自动管理生命周期。

**示例**
```
void someFunction(td::function<void()> CallBack) {
    int value = 42;
    QTimer::singleShot(100, [&]() {
        qDebug() << value; // 危险！value 可能已销毁
        CallBack();//[&] 捕获了临时变量，当回调触发时该对象可能已被销毁
    });
} // value 离开作用域被销毁，但 Lambda 可能稍后执行
```

+ 内存越界表现：
    - 尝试调用已释放的 std::function 导致 WASM 内存访问越界
    - 错误栈显示调用链在 std::function 操作时崩溃
    * 使用局部回调变量,或者智能指针


编程纪要
---------------------------------------
> Qt 支持 GET、POST 还是其他 HTTP 方法，body() 都可以调用，且不会抛出异常
