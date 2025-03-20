
# 设置可执行文件名称
set(PLUGIN_NAME ScannerFaro)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# 设置插件输出目录 ${CMAKE_BINARY_DIR} 表示构建目录（通常是 build/）。
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/plugins)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}) # 指定动态库的输出路径（Linux/macOS 的 .so/.dylib）。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR})# 可执行文件输出目录（Windows 下 .dll）

# 将源文件组织在变量中
set(CONTROLLER_SOURCES
    "${INCLUDE_DIR}interface/iPluginDevice.h"
)

# 使用file命令自动查找所有源文件
file(GLOB_RECURSE PROJECT_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

#INTERFACE
add_library(${PLUGIN_NAME} SHARED 
	${CONTROLLER_SOURCES}
	${PROJECT_SOURCES}
	ScannerPlugin.json
)

# 确保插件和应用依赖共享库
add_dependencies(${PLUGIN_NAME} South)

target_link_libraries(${PLUGIN_NAME} PRIVATE
	Qt${QT_VERSION_MAJOR}::Core
    South
)

#然后在CMake中启用预编译头
target_precompile_headers(${PLUGIN_NAME} PRIVATE pch.h)

#设置编译编译定义是在编译过程中传递给预处理器的符号，可以用来控制代码中的条件编译。
target_compile_definitions(${PLUGIN_NAME} PRIVATE
    PLUGIN_LIBRARY
    QT_PLUGIN
)
