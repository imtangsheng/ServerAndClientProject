# 设置可执行文件名称
set(SHARE_NAME South)

#include(qt.cmake)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(${SHARE_NAME} SHARED
  ${INCLUDE_DIR}South.h 
  South.cpp
)

target_link_libraries(${SHARE_NAME} PRIVATE
	Qt${QT_VERSION_MAJOR}::Core
)

target_compile_definitions(${SHARE_NAME} PRIVATE SHAREDLIB_API)
target_precompile_headers(${SHARE_NAME} PUBLIC stdafx.h)

#set_target_properties(${SHARE_NAME} PROPERTIES
#    DEBUG_POSTFIX "d"
#)