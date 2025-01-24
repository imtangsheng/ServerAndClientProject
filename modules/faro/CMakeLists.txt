cmake_minimum_required(VERSION 3.16)

project(TestImageServer LANGUAGES CXX)

# 获取环境变量South_Thirdparty
set(THIRD_PARTY_PATH E:/works/thirdparty)
if(NOT THIRD_PARTY_PATH)
    message(FATAL_ERROR "Please set South_Thirdparty environment variable")
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Network Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Network Concurrent)

# 启用 C++ 异常处理机制
if(MSVC)
    add_compile_options("/EHsc")
endif()

# 检查是否支持 OpenMP
find_package(OpenMP)
if(OPENMP_FOUND)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_executable(TestImageServer
    main.cpp
)

add_executable(Test 
    test.cpp
)


# 添加共享库
add_library(MySharedLib STATIC
  TaskHandle.h TaskHandle.cpp
  imageserver.h imageserver.cpp
  FaroHandle.h FaroHandle.cpp
  PointCloudImageGenerate.h PointCloudImageGenerate.cpp
  RealtimeSolving.h RealtimeSolving.cpp
)

# 为MySharedLib添加包含目录
include_directories(MySharedLib PUBLIC
    # 如果有其他自定义包含目录，可以在这里添加
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    #${THIRD_PARTY_PATH}/opencv/include
)

link_libraries(MySharedLib PUBLIC
    ${THIRD_PARTY_PATH}/opencv/x64/vc16/lib
)
# 指定共享库的链接库
target_link_libraries(MySharedLib PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Network
  Qt${QT_VERSION_MAJOR}::Concurrent
  #${THIRD_PARTY_PATH}/opencv/x64/vc16/lib/opencv_world480d.lib
)

# 链接共享库到执行文件
target_link_libraries(TestImageServer
  MySharedLib
)

target_link_libraries(Test
  MySharedLib
)
include(GNUInstallDirs)
install(TARGETS TestImageServer
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
