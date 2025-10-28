
# 设置可执行文件名称
set(ProgramName FaroPreview)

# Core核心需要包含
# Gui图像保存需要gui模块
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Concurrent)
# 启用 C++ 异常处理机制
if(MSVC)
    add_compile_options("/EHsc")
endif()
# 为MySharedLib添加包含目录
include_directories(${ProgramName} PUBLIC
    # 如果有其他自定义包含目录，可以在这里添加
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    #${THIRD_PARTY_PATH}/opencv/include
)

# 将源文件组织在变量中
set(ProgramSources
  logger.h logger.cpp
  TaskHandle.h TaskHandle.cpp
  FaroHandle.h FaroHandle.cpp
  PointCloudImageGenerate.h PointCloudImageGenerate.cpp
  RealtimeSolving.h RealtimeSolving.cpp
)

add_executable(${ProgramName}
    ${ProgramSources}
    main.cpp
)

# 指定共享库的链接库
target_link_libraries(${ProgramName} PUBLIC
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Concurrent
  #${THIRD_PARTY_PATH}/opencv/x64/vc16/lib/opencv_world480d.lib
)