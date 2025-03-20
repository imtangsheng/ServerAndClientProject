
# 设置可执行文件名称
set(PLUGIN_NAME CameraDvp)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON) #生成编译数据库（compile_commands.json）通过生成这个文件，IDE 可以正确解析预编译头的包含。

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

# 设置插件输出目录 ${CMAKE_BINARY_DIR} 表示构建目录（通常是 build/）。
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/plugins)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}) # 指定动态库的输出路径（Linux/macOS 的 .so/.dylib）。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR})# 可执行文件输出目录（Windows 下 .dll）

# 设置配置文件输出目录
set(CONFIG_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/config)

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
	CameraPlugin.json
)

# 确保插件和应用依赖共享库
add_dependencies(${PLUGIN_NAME} South)

target_link_libraries(${PLUGIN_NAME} PRIVATE
	Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    South
)

#然后在CMake中启用预编译头
target_precompile_headers(${PLUGIN_NAME} PRIVATE pch.h)

# 为目标添加编译器选项（适用于 GCC/Clang/MSVC）
#target_compile_options(${PLUGIN_NAME} PRIVATE
#    $<$<CXX_COMPILER_ID:GNU,Clang>:-include-pch ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PLUGIN_NAME}.dir/pch.h.pch>
#    $<$<CXX_COMPILER_ID:MSVC>:/FI${CMAKE_CURRENT_SOURCE_DIR}/pch.h>
#)
#库文件的链接路径设置为源代码目录的路径
target_link_directories(${PLUGIN_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/x64")
target_link_libraries(${PLUGIN_NAME} PRIVATE DVPCamera64)

#设置编译编译定义是在编译过程中传递给预处理器的符号，可以用来控制代码中的条件编译。
target_compile_definitions(${PLUGIN_NAME} PRIVATE
    PLUGIN_LIBRARY
    QT_PLUGIN
)

# 复制 JSON 配置文件到输出目录
#POST_BUILD 表示在目标构建完成后执行。
#使用 add_custom_command 在编译完成后将 config/CameraDvp.json 复制到 out/config/。
#make_directory 确保输出目录存在，copy_if_different 仅在文件有变化时复制。

#set(JSON_FILE ${CMAKE_SOURCE_DIR}/config/${PLUGIN_NAME}.json)
#add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E make_directory ${CONFIG_OUTPUT_DIR}
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        ${JSON_FILE}
#        ${CONFIG_OUTPUT_DIR}/${PLUGIN_NAME}.json
#    COMMENT "Copying ${PLUGIN_NAME}.json to output directory"
#)