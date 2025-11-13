cmake_minimum_required(VERSION 3.16)

project(ProjectWebClient VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/bin)  # 可执行文件输出目
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/libs)  # 动态库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/libs/lib)  # 静态库输出目录

set(ProjectPath ../../)
include_directories(${ProjectPath})
include_directories(${ProjectPath}/include)
##动态库依赖
set(SHARE_SOURCES
  ${ProjectPath}include/share_lib.h ${ProjectPath}include/logger.h
  ${ProjectPath}libs/share/share_lib.cpp ${ProjectPath}libs/share/log/logger.cpp
)

##客户端
set(client CameraClient)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network WebSockets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network WebSockets LinguistTools)

set(PathClient "${ProjectPath}/main/client/")
include_directories(${PathClient})
set(CONTROLLER_FILES
    ${PathClient}MainControl.h ${PathClient}MainControl.cpp
    ${PathClient}ui/WebSocketWidget.h ${PathClient}ui/WebSocketWidget.cpp ${PathClient}ui/WebSocketWidget.ui
    ${PathClient}dialog/HttpDialog.h ${PathClient}dialog/HttpDialog.cpp ${PathClient}dialog/HttpDialog.ui
)
source_group("Controllers" FILES ${CONTROLLER_FILES})

set(TS_FILES ${client}_en_US.ts)
set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        ${TS_FILES} ${client}.qrc
)
qt_add_executable(${client}
    ${SHARE_SOURCES} ${CONTROLLER_FILES}
    ${PROJECT_SOURCES}
    readme.md

)

target_precompile_headers(${client} PRIVATE ${ProjectPath}libs/share/stdafx.h)

target_link_libraries(${client} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::WebSockets
)

# 添加更新翻译?
qt_add_lupdate(${client}
    TS_FILES ${TS_FILES}
    OPTIONS
    -no-obsolete        # 删除过时的翻?
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${client})
endif()
