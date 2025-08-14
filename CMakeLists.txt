cmake_minimum_required(VERSION 3.16)
# 定义项目名称和版本 
project(SouthProject VERSION 0.0.1 LANGUAGES CXX) #project() 会创建项目和VERSION版本的CMake 变量
# Qt配置
set(CMAKE_AUTOMOC ON)  # 启用Qt的MOC
set(CMAKE_AUTORCC ON)  # 启用Qt的资源编译
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141) #最低CMake版本要求3.25
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
endif()

# 打印版本信息
message("PROJECT_NAME: ${PROJECT_NAME}")
message("SouthProject_VERSION: ${SouthProject_VERSION}")
message("SouthProject_VERSION_MAJOR: ${SouthProject_VERSION_MAJOR}")
message("PROJECT_VERSION: ${PROJECT_VERSION}")
# 打印路径信息
message("Source dir: ${SouthProject_SOURCE_DIR}")
message("Binary dir: ${SouthProject_BINARY_DIR}")



# 设置全局 Debug 后缀
set(CMAKE_DEBUG_POSTFIX "d")
# 设置特定目标的 Debug 后缀
#set_target_properties(${PLUGIN_NAME} PROPERTIES
#    DEBUG_POSTFIX "d"
#)

#set(CMAKE_PREFIX_PATH "E:/Qt/6.8.2/msvc2022_64") # Qt Kit Dir for vscode
# 添加cmake模块路径 默认只在 CMAKE_MODULE_PATH 中查找模块，而当前目录不在 CMAKE_MODULE_PATH 中。
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

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
