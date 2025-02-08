# 指定最低CMake版本要求
cmake_minimum_required(VERSION 3.16)
# 定义项目名称和版本
project(AcquisitionProject VERSION 1.0.0 LANGUAGES CXX)

# 添加cmake模块路径
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/main)
# 包含通用设置和依赖
include(ProjectSettings)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译器选项
if(MSVC)
    add_compile_options(/W4)    # Windows下的警告级别
else()
    add_compile_options(-Wall)  # Unix下的警告级别
endif()

# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core WebSockets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core WebSockets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_PREFIX_PATH "E:/Qt/6.8.2/msvc2022_64") # Qt Kit Dir for vscode
# 启用 Qt 的自动 moc、rcc 和 uic
# set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(${PROJECT_SOURCE_DIR}/include)






# 设置库名称
set(LIB_NAME SouthLib)
# 创建动态库
add_library(${LIB_NAME} STATIC
 "include/network/WebSocketServer.h"
 "libs/network/WebSocketServer.cpp"
  "include/network/WebSocketClient.h" "libs/network/WebSocketClient.cpp")
# CMake无法从文件扩展名推断语言时,只有头文件没有源文件时,明确告诉CMake使用C++作为链接语言
#set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)

# 添加预处理器定义，用于Windows DLL导出
target_compile_definitions(${LIB_NAME} PRIVATE CALCULATOR_EXPORTS)

# 获取/include目录及其子目录下的所有文件夹
file(GLOB INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

# 将所有获取到的目录添加到包含目录
foreach(DIR ${INCLUDE_DIRS})
    if(IS_DIRECTORY ${DIR})
        target_include_directories(${LIB_NAME} PUBLIC ${DIR})
    endif()
endforeach()

# 链接 Qt 库
target_link_libraries(${LIB_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::WebSockets
)


# add_subdirectory(include)
# 添加子项目
#add_subdirectory(main)
add_subdirectory(modules)
#add_subdirectory(devices)
#add_subdirectory(test)
#add_subdirectory(libs/SouthStaticLib)