# 设置可执行文件名称 移动到最顶层中定义
#set(SHARE_NAME Shared)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# 使用file命令自动查找所有源文件
file(GLOB_RECURSE PROJECT_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

#需要包含头文件 继承QObject的类
add_library(${SHARE_NAME} SHARED
  stdafx.h
  ${INCLUDE_DIR}shared.h ${INCLUDE_DIR}Logger.h ${INCLUDE_DIR}WorkHandler.h
  ${PROJECT_SOURCES}
)

target_link_libraries(${SHARE_NAME} PRIVATE
	Qt${QT_VERSION_MAJOR}::Core
)

target_compile_definitions(${SHARE_NAME} PRIVATE SHAREDLIB_API)
target_precompile_headers(${SHARE_NAME} PUBLIC stdafx.h)

#set_target_properties(${SHARE_NAME} PROPERTIES
#    DEBUG_POSTFIX "d"
#)
