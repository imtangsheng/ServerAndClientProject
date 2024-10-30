project(demo LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_library(demo SHARED
  plugindemo.cpp
  plugindemo.h
  demo.json
  ../../include/interface/QtPluginDeviceInterface.h
)

target_link_libraries(demo PRIVATE Qt${QT_VERSION_MAJOR}::Core)

target_compile_definitions(demo PRIVATE DEMO_LIBRARY)
