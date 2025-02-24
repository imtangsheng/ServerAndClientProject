# 设置可执行文件名称
set(APP_NAME QTROServer)


# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core RemoteObjects WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core RemoteObjects WebSockets)

add_executable(${APP_NAME} main.cpp websocketiodevice.h websocketiodevice.cpp)

# 链接 Qt 库
target_link_libraries(${APP_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::RemoteObjects
    Qt${QT_VERSION_MAJOR}::WebSockets
)