# 设置可执行文件名称 移动到最顶层中定义
#set(SHARE_NAME Shared)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# 使用file命令自动查找所有源文件
file(GLOB_RECURSE PROJECT_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

#需要包含头文件 继承QObject的类
add_library(${SHARE_NAME} SHARED
    ${INCLUDE_DIR}shared.h ${INCLUDE_DIR}Logger.h ${INCLUDE_DIR}TaskManager.h
    #${INCLUDE_DIR}common/FileReadAndSave.ixx
    ${PROJECT_SOURCES}
)

target_link_libraries(${SHARE_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)

#添加版本号信息
target_compile_definitions(${SHARE_NAME} PRIVATE 
    SHAREDLIB_API #导出标志
    VERSION_STR="${PROJECT_VERSION}"
)

target_precompile_headers(${SHARE_NAME} PUBLIC stdafx.h)

