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

# 启用 Qt 的自动 moc、rcc 和 uic
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)

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
add_subdirectory(libs/share)
add_subdirectory(main/server)
#add_subdirectory(main/client)
add_subdirectory(main/MS301Client)
