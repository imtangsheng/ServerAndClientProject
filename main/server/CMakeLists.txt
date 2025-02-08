
# 设置可执行文件名称
set(APP_NAME server)

# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core WebSockets)

add_executable(${APP_NAME}
    main.cpp
    ${INCLUDE_DIR}common.h
    "${INCLUDE_DIR}network/WebSocketServer.h"
)

set_target_properties(${APP_NAME} PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

# 链接 Qt 库
target_link_libraries(${APP_NAME} PUBLIC
    SouthLib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::WebSockets
)