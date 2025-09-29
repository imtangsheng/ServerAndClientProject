# 设置插件输出目录 ${CMAKE_BINARY_DIR} 表示构建目录（通常是 build/）。
set(PLUGIN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/plugins)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR}) # 指定动态库的输出路径（Linux/macOS 的 .so/.dylib）。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGIN_OUTPUT_DIR})# 可执行文件输出目录（Windows 下 .dll）
# 设置配置文件输出目录
set(CONFIG_OUTPUT_DIR ${CMAKE_BINARY_DIR}/out/config)

add_subdirectory(plugin/Serial)
add_subdirectory(plugin/Scanner)
#add_subdirectory(plugin/Cameras)
