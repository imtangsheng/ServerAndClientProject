set(INTERFACE_NAME IncludeInterface)

#set(CMAKE_PREFIX_PATH "E:/Qt/6.9.0/msvc2022_64") # Qt Kit Dir

# 查找所需的 Qt 组件
#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core SerialPort)
#find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core SerialPort)

# 收集头文件
file(GLOB_RECURSE PROJECT_HEADERS 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.ixx
)

# 添加接口库 接口库无法被链接到可执行文件或共享库中,包含文件是找不到链接头文件的
add_library(${INTERFACE_NAME} INTERFACE
    ${PROJECT_HEADERS}
)

#target_precompile_headers(${INTERFACE_NAME} INTERFACE stdafx.h)
# 链接Qt库
#target_link_libraries(${INTERFACE_NAME} INTERFACE
#    Qt${QT_VERSION_MAJOR}::Core
#    Qt${QT_VERSION_MAJOR}::SerialPort
#    ${CMAKE_CURRENT_SOURCE_DIR}
#)

#target_link_libraries(server PRIVATE ${INTERFACE_NAME})