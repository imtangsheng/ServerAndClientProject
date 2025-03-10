set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/bin)  # 可执行文件输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/libs/dll)  # 动态库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/libs/lib)  # 静态库输出目录

add_subdirectory(plugin/Trolley)
add_subdirectory(plugin/ScannerFaro)
add_subdirectory(plugin/CameraDvp)

#用于指定目标之间的构建依赖关系。它确保 dependee（被依赖的目标，South）在 target（Server）构建之前完成编译。
add_dependencies(CameraDvp South)