
# 设置可执行文件名称
set(PLUGIN_NAME Faro)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network) 
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network)

# 设置插件输出目录 ${CMAKE_BINARY_DIR} 表示构建目录（通常是 build/）。
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/plugins)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}) # 指定动态库的输出路径（Linux/macOS 的 .so/.dylib）。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR})# 可执行文件输出目录（Windows 下 .dll）

# 将源文件组织在变量中
set(PLUGIN_SOURCES
    "${INCLUDE_DIR}interface/iPluginDevice.h"
    ScannerPlugin.h ScannerPlugin.cpp
    ScannerPlugin.json "readme.md"
    Faro/FaroControl.h Faro/FaroControl.cpp
    Faro/FaroHandle.h Faro/FaroHandle.cpp
)

# 使用file命令自动查找所有源文件
#file(GLOB_RECURSE PROJECT_SOURCES 
#    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
#    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
#)

#INTERFACE
add_library(${PLUGIN_NAME} SHARED 
	${PLUGIN_SOURCES}
    "${PLUGIN_NAME}.qrc"
)
qt_add_resources(${PLUGIN_NAME} "${PLUGIN_NAME}.qrc")

# 确保插件和应用依赖共享库
add_dependencies(${PLUGIN_NAME} ${SHARE_NAME})

target_link_libraries(${PLUGIN_NAME} PRIVATE
	Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    ${SHARE_NAME}
)

#设置编译编译定义是在编译过程中传递给预处理器的符号，可以用来控制代码中的条件编译。
target_compile_definitions(${PLUGIN_NAME} PRIVATE
    DEVICE_TYPE_SCAN_FARO
)

#自动生成翻译文本
#qt_add_lupdate(${PLUGIN_NAME}
#    TS_FILES ${PROJECT_SOURCE_DIR}/assets/translations/${PLUGIN_NAME}_zh_CN.ts
#    OPTIONS
#    -no-obsolete        # 删除过时的翻译
#)
