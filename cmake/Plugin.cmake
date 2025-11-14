# 设置插件输出目录 ${CMAKE_BINARY_DIR} 表示构建目录（通常是 build/）。
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/plugins)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}) # 指定动态库的输出路径（Linux/macOS 的 .so/.dylib）。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR})# 可执行文件输出目录（Windows 下 .dll）
# 设置配置文件输出目录
set(CONFIG_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/config)

add_subdirectory(plugin/Serial)
add_subdirectory(plugin/Scanner)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # 这里添加 Debug 模式下的特定代码
    add_subdirectory(plugin/Cameras)
else()
    # 这里添加 Release 或其他模式下的代码
endif()

