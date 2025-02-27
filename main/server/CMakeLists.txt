# 设置可执行文件名称
set(APP_NAME server)
# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core WebSockets)

# 将源文件组织在变量中
set(NETWORK_SOURCES
    network/WebSocketServer.h
    network/WebSocketServer.cpp
)

set(CONTROLLER_SOURCES
    manager/ManagerPlugin.h manager/ManagerPlugin.cpp
)

# 创建可执行文件
add_executable(${APP_NAME}
    stdafx.h main.cpp
    ${NETWORK_SOURCES}
    ${CONTROLLER_SOURCES}
 )
# 启用预编译头文件 必须在目标（如 add_executable 或 add_library）定义之后使用。
target_precompile_headers(${APP_NAME} PRIVATE stdafx.h)
# 设置目标属性
set_target_properties(${APP_NAME} PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

# 链接 Qt 库
target_link_libraries(${APP_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::WebSockets
    South
)
