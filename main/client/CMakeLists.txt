# 设置可执行文件名称
set(APP_NAME client)

# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools WebSockets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# 按类别组织源文件
set(CORE_FILES
    MainController.h MainController.cpp
)

set(UI_FILES
    ui/WebSocketWidget.h ui/WebSocketWidget.cpp ui/WebSocketWidget.ui
    ui/ImageViewer.h ui/ImageViewer.cpp ui/ImageViewer.ui
    ui/ScannerWidget.h ui/ScannerWidget.cpp ui/ScannerWidget.ui
    ui/CameraWidget.h ui/CameraWidget.cpp ui/CameraWidget.ui

)

# 设置翻译文件
set(TS_FILES_
    ${PROJECT_SOURCE_DIR}/translations/${APP_NAME}_zh_CN.ts
)


set(CONTROLLER_FILES
    #${INCLUDE_DIR}controller/CameraController.h controller/CameraController.cpp
    #controller/user_controller.h controller/user_controller.cpp
)

# 创建文件夹结构以便在IDE中组织文件
source_group("Core" FILES ${CORE_FILES})
source_group("UI" FILES ${UI_FILES})
source_group("Controllers" FILES ${CONTROLLER_FILES})
# 使用file命令自动查找所有源文件
file(GLOB_RECURSE PROJECT_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/*.css
)

# 添加可执行文件并包含所有源文件
add_executable(${APP_NAME}
    stdafx.h main.cpp "resources.qrc"
    ${PROJECT_SOURCES}
    ${CORE_FILES}
    ${UI_FILES} ${TS_FILES_}
    ${CONTROLLER_FILES}
)
# 启用预编译头文件 必须在目标（如 add_executable 或 add_library）定义之后使用。
target_precompile_headers(${APP_NAME} PRIVATE stdafx.h)

# CMakeLists.txt 中添加
qt_add_resources(${APP_NAME} "resources.qrc")

qt_add_lupdate(${APP_NAME}
    TS_FILES ${TS_FILES_}
    OPTIONS
    -no-obsolete        # 删除过时的翻译
)
#qt_add_lrelease(${APP_NAME}
#    TS_FILES ${TS_FILES}
#    QM_FILES_OUTPUT_VARIABLE qm_files
#)
# 自动更新ts文件
#qt_create_translation(QM_FILES "mainwindow.ui" ${TS_FILES})

# 链接 Qt 库
target_link_libraries(${APP_NAME} PRIVATE
    South
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebSockets
)

# 安装翻译文件
install(FILES ${qm_files}
    DESTINATION translations
    COMPONENT Runtime
)
