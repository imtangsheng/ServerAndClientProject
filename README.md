## 命名风格总纲
### 文件命名
    Qt:头文件,源文件使用 PascalCase。(UI文件和继承QObject,QWidget的类)
    Qt:资源文件：使用 snake_case。
    一般文件:文件名使用小写字母，单词间用下划线连接 snake_case

### 成员变量
    私有成员变量：推荐使用 m_ 前缀。
    公有和保护成员变量：通常不加前缀，除非是 Qt 对象指针。使用 snake_case。

后缀:
- **下划线**:当成员变量与构造函数参数和关键字同名时，使用后缀下划线.函数使用前缀下划线,当是getter函数使用,使用 snake_case。
- **ptr**:当成员变量是指针时，使用 ptr 后缀
- **ref**:当成员变量是引用时，使用 ref 后缀

### 特别约定

- **常用方法**:常用的方法使用小写替换,如init,get,set等常规方法
- **信号**:使用signal_前缀的snake_case或者sig前缀的camelCase
- **槽**:使用on_ slot_前缀的snake_case或者on handle slot 前缀的camelCase

### 注意事项
- **公有槽函数**:QString类型和结构体参数使用const type& 的引用(引用计数析构),普通类型如int double使用值传递(不得使用引用,数据可能会被立即销毁)

# 采集软件开发

Develop: Qt6 C++ cmake VS2022 QtCreator

# 适配多个Qt5的版本
    1.使用vs2022
        1)打开 VS2022
        2)进入菜单：扩展 -> Qt VS Tools -> Qt Versions
        3)在打开的窗口中添加不同的Qt版本路径
        4)选择默认的qt版本
        5)清理删除Cmake缓存并重新构建项目(CMake预设文件(CMakePresets.json)的配置会更新,QTDIR：Qt安装路径)
        6)重新设置版本从4)开始,默认版本 并删除CMake 缓存

# C++ 命名规范手册
## 1. 文件命名
    头文件：.h, .hpp
    源文件：.cpp
    文件名使用小写字母，单词间用下划线连接
    示例：string_utils.hpp, file_manager.cpp
## 2. 命名空间 (Namespace)
    使用小写字母
    避免使用下划线
    示例：

```cpp
namespace microsoft {
namespace system {
namespace tools {
    // 代码
}
}
}
```
## 3. 类命名 (Class)
    使用 Pascal 命名法（每个单词首字母大写）
    不使用前缀
    示例：

```cpp
class FileManager;
class StringUtils;
class NetworkConnection;
```
## 4. 函数命名 (Function)
    使用 Pascal 命名法
    动词或动词短语
    示例：
```cpp
void ConnectToServer();
bool IsValidPath();
string GetUserName();
```
## 5. 变量命名 (Variables)
### 5.1 成员变量
    使用 m_ 前缀
    后跟 Pascal 命名法
    示例：
```cpp
class Example {
private:
    int m_Count;
    string m_Name;
    bool m_IsValid;
};
```
### 5.2 局部变量
    使用驼峰命名法（首字母小写）
    示例：
```cpp
void Function() {
    int itemCount;
    string userName;
    bool isValid;
}
```
### 5.3 常量
    全大写，单词间用下划线连接
    示例：
```cpp
const int MAX_COUNT = 100;
const string DEFAULT_PATH = "C:/";
```
## 6. 接口命名 (Interface)
    使用 I 前缀
    后跟 Pascal 命名法
    示例：
```cpp
class IConnectable;
class IDisposable;
class IRunnable;
```
## 7. 枚举命名 (Enum)
### 7.1 枚举类型
    使用 Pascal 命名法
    示例：
```cpp
enum Color;
enum FileType;
```
### 7.2 枚举值
    使用 Pascal 命名法
    全大写，单词间用下划线连接
    示例：
```cpp
enum Color {
    COLOR_RED,
    Red,
    Blue,
    Green
};
```
## 8. 模板参数命名 (Template Parameters)
    使用 Pascal 命名法
    单个字母时使用 T 作为前缀
    示例：
```cpp
template<typename TKey, typename TValue>
class Dictionary;

template<typename T>
class List;
```
## 9. 宏命名 (Macros)
    全大写，单词间用下划线连接
    示例：
```cpp
#define MAX_BUFFER_SIZE 1024
#define SAFE_DELETE(p) if(p) { delete p; p = nullptr; }
```
## 10. 代码格式规范
### 10.1 缩进
    使用4个空格缩进
    不使用制表符（Tab）
### 10.2 大括号
    左大括号在行尾
    右大括号独占一行
    示例：
```cpp
if (condition) {
    // 代码
}

class Example {
public:
    Example();
private:
    int m_Value;
};
```
### 10.3 空格使用
    运算符前后添加空格
    逗号后添加空格
    示例：
```cpp
int sum = a + b;
Function(param1, param2, param3);
```
## 11. 注释规范
### 11.1 文件头注释
```cpp
/**
 * @file filename.hpp
 * @brief 文件简述
 * @author 作者名
 * @date 2025-03-10
 */
 ```
### 11.2 函数注释
```cpp
/**
 * @brief 函数简述
 * @param param1 参数1说明
 * @param param2 参数2说明
 * @return 返回值说明
 * @throw 异常说明
 */
 ```
### 11.3 类注释
```cpp
/**
 * @brief 类的简述
 * @details 详细说明
 */
 ```
### 11.4 行注释
```cpp
// 单行注释使用双斜线

/* 多行注释
   使用星号 */
```
## 12. 最佳实践建议
    命名应该具有描述性，避免使用缩写
    一个函数只做一件事，函数名应当准确描述其功能
    变量名应反映其内容，而不是类型
    避免使用匈牙利命名法
    保持命名风格的一致性
    使用有意义的名称，避免无意义的名称如 a, b, c

# Qt 框架
## 1. Qt 类型命名规则
    Qt 类型本身：Qt 类型的名称已经是 PascalCase（如 QString、QList），直接使用即可。
    Qt 类型的变量：变量名使用 camelCase，与微软规范一致。
## 2. Qt 信号和槽(此处使用默认,小写_下划线)
### 信号 (Signals)：
    信号名称使用 snake_case，并以动词开头，表示某种事件或状态变化。
    信号名称通常以 ed 结尾，表示已完成的动作。
```cpp
void buttonClicked();
void dataLoaded();
void progressChanged(int value);
```

### 槽 (Slots)：
    槽函数名称使用 snake_case，并以动词开头，表示对信号的处理。
```cpp
void onButtonClicked();
void handleDataLoaded();
void updateProgress(int value);
```

## 3. Qt 对象指针
    前缀 m_：如果是指向 Qt 对象的成员变量，使用 m_ 前缀。
```cpp
QPushButton* m_submitButton;
QLabel* m_statusLabel;
```
    局部变量：局部变量不需要前缀，直接使用 camelCase。

```cpp
QPushButton* submitButton = new QPushButton(this);
QLabel* statusLabel = new QLabel(this);
```
## 4. Qt 容器类型
    变量命名：Qt 容器类型（如 QList、QMap、QVector 等）的变量名应描述其内容。
```cpp
QList<QString> userNames;
QMap<int, QString> userIdToNameMap;
QVector<int> scores;
```
## 5. Qt 枚举
    枚举类型：使用 PascalCase。
```cpp
enum class FileStatus {
    Open,
    Closed,
    Error
};
```
    枚举值：使用 PascalCase。
```cpp
FileStatus status = FileStatus::Open;
```
## 6. Qt 自定义控件
    控件类名：使用 PascalCase，并以控件类型结尾（如 Button、Dialog、Widget）。

```cpp
class CustomButton : public QPushButton {
    // ...
};

class SettingsDialog : public QDialog {
    // ...
};
```
## 7. Qt 资源文件
    资源文件命名：使用 snake_case。
```
icons/button_icon.png
styles/main_style.qss
```
## 8. Qt 信号和槽连接
    命名一致性：信号和槽的名称应保持一致，便于理解。
```cpp
connect(m_submitButton, &QPushButton::clicked, this, &MainWindow::onSubmitButtonClicked);
```
## 9. Qt 文件命名
    头文件和源文件：使用 PascalCase。
```
MainWindow.h
MainWindow.cpp
UI 文件：使用 snake_case。(使用默认即可MainWindow.ui)
main_window.ui
settings_dialog.ui
```
## 10. 示例代码
    以下是一个结合微软规范和 Qt 惯例的示例：
```cpp
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QList>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void onSubmitButtonClicked();
    void onDataLoaded(const QList<QString>& data);

private:
    void initializeUi();
    void loadData();

    QPushButton* m_submitButton;
    QLabel* m_statusLabel;
    QList<QString> m_dataList;
};

// 实现
MainWindow::MainWindow(QWidget* parent) : QWidget(parent) {
    initializeUi();
    loadData();
}

void MainWindow::initializeUi() {
    m_submitButton = new QPushButton("Submit", this);
    m_statusLabel = new QLabel("Ready", this);

    connect(m_submitButton, &QPushButton::clicked, this, &MainWindow::onSubmitButtonClicked);
}

void MainWindow::loadData() {
    m_dataList = {"Item1", "Item2", "Item3"};
    emit onDataLoaded(m_dataList);
}

void MainWindow::onSubmitButtonClicked() {
    m_statusLabel->setText("Submitted");
}

void MainWindow::onDataLoaded(const QList<QString>& data) {
    // 处理数据
}
```
