set(INTERFACE_NAME MyCore)

#set(CMAKE_PREFIX_PATH "e:/Qt/6.7.2/msvc2019_64") # Qt Kit Dir

# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# 收集头文件
file(GLOB_RECURSE PROJECT_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# 添加接口库
add_library(${INTERFACE_NAME} INTERFACE
    ${PROJECT_HEADERS}
)

#target_precompile_headers(${INTERFACE_NAME} INTERFACE stdafx.h)
# 链接Qt库
target_link_libraries(${INTERFACE_NAME} INTERFACE
    Qt${QT_VERSION_MAJOR}::Core
)