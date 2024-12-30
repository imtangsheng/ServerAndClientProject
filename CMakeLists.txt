cmake_minimum_required(VERSION 3.14)
project(SouthProject LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_PREFIX_PATH "e:/Qt/6.8.1/msvc2022_64") # Qt Kit Dir
# 启用 Qt 的自动 moc、rcc 和 uic
# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specify MSVC UTF-8 encoding   
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include_directories(${PROJECT_SOURCE_DIR}/include)

# add_subdirectory(include)

add_subdirectory(main)
add_subdirectory(devices)
add_subdirectory(test)
add_subdirectory(libs/SouthStaticLib)
