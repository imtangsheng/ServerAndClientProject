# 指定最低CMake版本要求
cmake_minimum_required(VERSION 3.16)
# 定义项目名称和版本
project(AcquisitionProject VERSION 1.0.0 LANGUAGES CXX)
# 设置全局 Debug 后缀
set(CMAKE_DEBUG_POSTFIX "d")
# 设置特定目标的 Debug 后缀
#set_target_properties(${PLUGIN_NAME} PROPERTIES
#    DEBUG_POSTFIX "d"
#)

#set(CMAKE_PREFIX_PATH "E:/Qt/6.8.2/msvc2022_64") # Qt Kit Dir for vscode
# 添加cmake模块路径
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/main)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /ZI")#启用 "编辑并继续" 功能的编译器选项
# 包含通用设置和依赖
include(ProjectSettings)

# add_subdirectory(include)
# 添加子项目
#add_subdirectory(main)
#add_subdirectory(modules)
#add_subdirectory(devices)
#add_subdirectory(example)
add_subdirectory(libs/South)
add_subdirectory("public/SouthStaticLib")
add_subdirectory(plugin/CameraDvp)
