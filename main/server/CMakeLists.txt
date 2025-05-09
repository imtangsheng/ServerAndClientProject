# 设置可执行文件名称
set(APP_NAME server)
# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core WebSockets HttpServer LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core WebSockets HttpServer LinguistTools)


# 将源文件组织在变量中
set(NETWORK_SOURCES
    network/WebSocketServer.h network/WebSocketServer.cpp
    network/HttpServer.h network/HttpServer.cpp
)

set(CONTROLLER_SOURCES
    manager/ManagerPlugin.h manager/ManagerPlugin.cpp
    UserServer.h UserServer.cpp
    #stdafx.h
)
# 设置翻译文件
set(TS_FILES_
    ${PROJECT_SOURCE_DIR}/assets/translations/${APP_NAME}_zh_CN.ts
)
# 创建文件夹结构以便在IDE中组织文件
source_group("network" FILES ${NETWORK_SOURCES})
source_group("control" FILES ${CONTROLLER_SOURCES})

# 创建可执行文件
add_executable(${APP_NAME}
    main.cpp "${APP_NAME}.qrc"
    ${NETWORK_SOURCES}
    ${CONTROLLER_SOURCES}
    ${TS_FILES_}
)
# 启用预编译头文件 必须在目标（如 add_executable 或 add_library）定义之后使用。
#target_precompile_headers(${APP_NAME} PRIVATE stdafx.h)
# 添加文本翻译文件
qt_add_resources(${APP_NAME} ${APP_NAME}.qrc)
#自动生成翻译文本
if(0)
qt_add_lupdate(${APP_NAME}
    TS_FILES ${TS_FILES_}
    OPTIONS
    -no-obsolete        # 删除过时的翻译
)

endif()
# 设置目标属性
set_target_properties(${APP_NAME} PROPERTIES
    WIN32_EXECUTABLE FALSE
    MACOSX_BUNDLE FALSE
)

# 链接 Qt 库
target_link_libraries(${APP_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::HttpServer
    ${SHARE_NAME}
)
