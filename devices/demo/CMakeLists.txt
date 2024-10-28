cmake_minimum_required(VERSION 3.16)

project(demo LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Gui)

add_library(demo SHARED
  plugindemo.cpp
  plugindemo.h
  demo.json
  ../../include/interface/QtPluginDeviceInterface.h
)

target_link_libraries(demo PRIVATE Qt${QT_VERSION_MAJOR}::Gui)

target_compile_definitions(demo PRIVATE DEMO_LIBRARY)
