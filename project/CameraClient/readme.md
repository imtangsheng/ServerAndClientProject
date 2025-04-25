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

开始构建编程
======================================
- bullet
+ other bullet
* another bullet
    * child bullet

1. ordered
2. next ordered

> A blockquote
> starts with >
>
> and has the same paragraph rules as normal text.
Paragraph.

Second Level Heading in Alternate Style
---------------------------------------
