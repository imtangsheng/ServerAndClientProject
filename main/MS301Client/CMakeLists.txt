# MS301客户端程序 TCC 隧影
set(APP_NAME TCC)

# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Charts LinguistTools WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Charts LinguistTools WebSockets)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
# 按类别组织源文件
set(CORE_FILES
    MainControl.h MainControl.cpp
)
#Custom drawing自定义的绘制控件
set(CustomUI_FILES
    ${PROJECT_SOURCE_DIR}/public/widget/LineChartView.h ${PROJECT_SOURCE_DIR}/public/widget/LineChartView.cpp
    #${PROJECT_SOURCE_DIR}/public/widget/AlarmToolButton.h ${PROJECT_SOURCE_DIR}/public/widget/AlarmToolButton.cpp

)

# 设置翻译文件
set(TS_FILES_
    assets/i18n/en_US.ts
)


# 创建文件夹结构以便在IDE中组织文件
source_group("Core" FILES ${CORE_FILES})
source_group("CustomUI" FILES ${CustomUI_FILES})

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
    ${CustomUI_FILES} ${TS_FILES_}
)
# 启用预编译头文件 必须在目标（如 add_executable 或 add_library）定义之后使用。
target_precompile_headers(${APP_NAME} PRIVATE stdafx.h)

# CMakeLists.txt 中添加
qt_add_resources(${APP_NAME} "resources.qrc")
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
