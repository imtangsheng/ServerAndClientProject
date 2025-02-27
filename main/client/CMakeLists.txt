# 设置可执行文件名称
set(APP_NAME client)

# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets WebSockets)

set(CMAKE_AUTOUIC ON)

add_executable(${APP_NAME}
    main.cpp
    MainController.h MainController.cpp
    MainView.h MainView.cpp
    ui/WebSocket.h ui/WebSocket.cpp ui/WebSocket.ui
    ui/ImageViewer.h ui/ImageViewer.cpp ui/ImageViewer.ui
    ui/farowidget.h ui/farowidget.cpp ui/farowidget.ui
    controller/farocontroller.h controller/farocontroller.cpp
)

# 链接 Qt 库
target_link_libraries(${APP_NAME} PRIVATE
    SouthLib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebSockets
)
