#!!!使用vs2022无法处理中文路径的文件资源
#set(CMAKE_UTF8 ON)  # 让 CMake 用 UTF-8 编译
# MS301客户端程序 TCC 隧影
set(APP_NAME TCC)

# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Charts LinguistTools WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Charts LinguistTools WebSockets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# 按类别组织源文件
set(PathPublic  ${PROJECT_SOURCE_DIR}/public/gui)
include_directories(${PathPublic})
#Custom drawing自定义的绘制控件
# 使用file命令自动查找所有源文件
file(GLOB_RECURSE UI_FILES
    ${PathPublic}/*/*.h ${PathPublic}/*/*.cpp ${PathPublic}/*/*.ui ${PathPublic}/ui.qrc
)

# 设置翻译文件
set(TS_FILES_
    ${PROJECT_SOURCE_DIR}/assets/translations/${APP_NAME}_zh_CN.ts
)

set(DEVICE_FILES
    ${PathPublic}/ChildWindow/ChildWindow.h
    ${PathPublic}/ChildWindow/TrolleyWidget.h ${PathPublic}/ChildWindow/TrolleyWidget.cpp ${PathPublic}/ChildWindow/TrolleyWidget.ui
    ${PathPublic}/ChildWindow/ScannerWidget.h ${PathPublic}/ChildWindow/ScannerWidget.cpp ${PathPublic}/ChildWindow/ScannerWidget.ui
    ${PathPublic}/ChildWindow/CameraWidget.h ${PathPublic}/ChildWindow/CameraWidget.cpp ${PathPublic}/ChildWindow/CameraWidget.ui
)

set(CORE_FILES
    core/MainControl.h core/MainControl.cpp
    core/ChildWidget.cpp
)

# 设置翻译文件
set(TS_FILES_
    assets/i18n/en_US.ts
)

# 创建文件夹结构以便在IDE中组织文件
source_group("Core" FILES ${CORE_FILES})
source_group("CustomUI" FILES ${UI_FILES})
source_group("DeviceUI" FILES ${DEVICE_FILES})
# 使用file命令自动查找所有源文件
file(GLOB_RECURSE PROJECT_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ui
)

# 添加可执行文件并包含所有源文件
add_executable(${APP_NAME}
    stdafx.h main.cpp "resources.qrc"
    ${PROJECT_SOURCES}
    ${CORE_FILES}
    ${TS_FILES_}
    ${UI_FILES}
)
# 启用预编译头文件 必须在目标（如 add_executable 或 add_library）定义之后使用。预编译头文件自动包含到所有源文件中,相当于在每个 .cpp 文件的开头隐式添加了 #include "stdafx.h"
target_precompile_headers(${APP_NAME} PRIVATE stdafx.h)

# CMakeLists.txt 中添加
qt_add_resources(${APP_NAME} "resources.qrc" ${PathPublic}/ui.qrc)
#自动生成翻译文本
if(0)
qt_add_lupdate(${APP_NAME}
    TS_FILES ${TS_FILES_}
    OPTIONS
    -no-obsolete        # 删除过时的翻译
)
endif()

#qt_add_lrelease(${APP_NAME}
#    TS_FILES ${TS_FILES}
#    QM_FILES_OUTPUT_VARIABLE qm_files
#)
# 自动更新ts文件
#qt_create_translation(QM_FILES "mainwindow.ui" ${TS_FILES})

# 链接 Qt 库
target_link_libraries(${APP_NAME} PRIVATE
    ${SHARE_NAME}
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::WebSockets
    Qt${QT_VERSION_MAJOR}::Charts
)

# 安装翻译文件
install(FILES ${qm_files}
    DESTINATION translations
    COMPONENT Runtime
)

# 设置目标属性
set_target_properties(${APP_NAME} PROPERTIES
    OUTPUT_NAME "${APP_NAME}${PROJECT_VERSION}" # 修改输出文件名
)

target_compile_definitions(${APP_NAME} PRIVATE
    __DEVICE_TYPE__="MS301"#三叶草相机设备类型的控制串口
)
