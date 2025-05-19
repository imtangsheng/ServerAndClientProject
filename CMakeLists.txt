cmake_minimum_required(VERSION 3.16) # 指定最低CMake版本要求
# 定义项目名称和版本 
project(AcquisitionProject VERSION 0.0.1 LANGUAGES CXX) #project() 会创建项目和VERSION版本的CMake 变量
# 打印版本信息
message("PROJECT_NAME: ${PROJECT_NAME}")
message("AcquisitionProject_VERSION: ${AcquisitionProject_VERSION}")
message("AcquisitionProject_VERSION_MAJOR: ${AcquisitionProject_VERSION_MAJOR}")
message("PROJECT_VERSION: ${PROJECT_VERSION}")
# 打印路径信息
message("Source dir: ${AcquisitionProject_SOURCE_DIR}")
message("Binary dir: ${AcquisitionProject_BINARY_DIR}")

# 设置全局 Debug 后缀
set(CMAKE_DEBUG_POSTFIX "d")
# 设置特定目标的 Debug 后缀
#set_target_properties(${PLUGIN_NAME} PROPERTIES
#    DEBUG_POSTFIX "d"
#)

#set(CMAKE_PREFIX_PATH "E:/Qt/6.8.2/msvc2022_64") # Qt Kit Dir for vscode
# 添加cmake模块路径 默认只在 CMAKE_MODULE_PATH 中查找模块，而当前目录不在 CMAKE_MODULE_PATH 中。
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /ZI")#启用 "编辑并继续" 功能的编译器选项
# 包含通用设置和依赖

# 设置可执行文件名称
set(SHARE_NAME Shared)

include(ProjectSettings)
#添加插件目录
include(Plugin)
add_subdirectory("include")
# 添加子项目
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/bin)  # 重新设置可执行文件输出目录
add_subdirectory(example)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # 这里添加 Debug 模式下的特定代码
    #add_dependencies(server )
else()
    
    # 这里添加 Release 或其他模式下的代码
endif()
