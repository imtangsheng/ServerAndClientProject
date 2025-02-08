# 设置编译器选项
if(MSVC)
    add_compile_options(/W4 /MP)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 设置默认的构建类型
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif()

#${CMAKE_CURRENT_SOURCE_DIR}：当前CMakeLists.txt所在目录
#${CMAKE_BINARY_DIR}：构建目录
# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/bin)  # 可执行文件输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/libs/dll)  # 动态库输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out/libs/lib)  # 静态库输出目录

# Qt设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 版本信息
set(PROJECT_VERSION_MAJOR 1)
set(PROJECT_VERSION_MINOR 0)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# 通用包含目录
set(COMMON_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)
# 获取/include目录及其子目录下的所有文件夹
file(GLOB INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

# 将所有获取到的目录添加到包含目录
foreach(DIR ${INCLUDE_DIRS})
    if(IS_DIRECTORY ${DIR})
        include_directories(${DIR})
    endif()
endforeach()

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# 定义一个函数用于设置通用的目标属性
function(set_common_target_properties TARGET_NAME)
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        LINKER_LANGUAGE CXX
    )
    
    # 添加通用的包含目录
    target_include_directories(${TARGET_NAME}
        PUBLIC
            ${COMMON_INCLUDE_DIRS}
    )
endfunction()


add_subdirectory(main/server)
add_subdirectory(main/client)