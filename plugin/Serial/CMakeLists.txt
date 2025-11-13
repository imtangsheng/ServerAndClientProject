
# 设置可执行文件名称
set(PLUGIN_NAME Serial)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core SerialPort)

#CMake会缓存option的值，即使你修改了CMakeLists.txt中的默认值，如果没有清理缓存，仍然会使用之前的值
#option(SUPPORT_CAR "Enable car device support" ON)
#option(SUPPORT_CAMERA "Enable clover camera device support" ON)
#option(SUPPORT_LINE_SCAN_CAMERA "Enable MS301 line scan camera device support" ON)
# 强制设置值（覆盖缓存）
#set(SUPPORT_CAR ON CACHE BOOL "Enable car device support" FORCE)
# 也可以使用set命令来设置变量的值
set(SUPPORT_CAR ON)
set(SUPPORT_CAMERA OFF)
set(SUPPORT_LINE_SCAN_CAMERA OFF)
message(STATUS "支持小车: ${SUPPORT_CAR}")
message(STATUS "支持相机: ${SUPPORT_CAMERA}")
message(STATUS "支持线阵相机:${SUPPORT_LINE_SCAN_CAMERA}")
# 定义后缀部分
set(NameSuffix "")#列表是用分号(;)分隔的字符串
if(SUPPORT_CAR)
    list(APPEND NameSuffix "Car")
endif()
if(SUPPORT_CAMERA)
    list(APPEND NameSuffix "Camera")
endif()
if(SUPPORT_LINE_SCAN_CAMERA)
    list(APPEND NameSuffix "LineScanCamera")
endif()
# 组合后缀 - 使用list(JOIN)
#list(JOIN NameSuffix "" DeviceNameSuffix)#连接列表元素时使用空格
string(JOIN "" DeviceNameSuffix ${NameSuffix}) #将分号连接在一起 (输出变量) (输入字符串，带有一个额外下划线))

# 将源文件组织在变量中
set(Serial_SOURCES
    "${INCLUDE_DIR}interface/iSerialPort.h"
    ${PROJECT_SOURCE_DIR}/public/serial/ActiveSerial.h ${PROJECT_SOURCE_DIR}/public/serial/ActiveSerial.cpp
    "${INCLUDE_DIR}interface/iPluginDevice.h"
    SerialPlugin.h SerialPlugin.cpp
    SerialPlugin.json "readme.md"
    ${PROJECT_SOURCE_DIR}/public/serial/LinsF400Imu.h)

#INTERFACE
add_library(${PLUGIN_NAME} SHARED 
    ${Serial_SOURCES}
    "${PLUGIN_NAME}.qrc"
)

# 修改输出文件名
set_target_properties(${PLUGIN_NAME} PROPERTIES
    OUTPUT_NAME "${PLUGIN_NAME}${DeviceNameSuffix}"
)

qt_add_resources(${PLUGIN_NAME} "${PLUGIN_NAME}.qrc")

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::SerialPort
    ${SHARE_NAME}
)

# 确保插件和应用依赖共享库
add_dependencies(${PLUGIN_NAME} ${SHARE_NAME})

#自动生成翻译文本
#qt_add_lupdate(${PLUGIN_NAME}
#    TS_FILES ${PROJECT_SOURCE_DIR}/assets/translations/${PLUGIN_NAME}_zh_CN.ts
#    OPTIONS
#    -no-obsolete        # 删除过时的翻译
#)

#设置编译编译定义是在编译过程中传递给预处理器的符号，可以用来控制代码中的条件编译。
target_compile_definitions(${PLUGIN_NAME} PRIVATE
    #__DEVICE_TYPE_MS301__ # 设备类型定义,新设备使用新的协议和受支持的命令
    __DEVICE_TYPE_MS201__ #
    $<$<BOOL:${SUPPORT_CAR}>:DEVICE_TYPE_CAR=1> #小车中控箱设备控制的串口控制板
    $<$<BOOL:${SUPPORT_CAMERA}>:DEVICE_TYPE_CAMERA=2> #三叶草相机设备类型的控制串口
    $<$<BOOL:${SUPPORT_LINE_SCAN_CAMERA}>:DEVICE_TYPE_LINE_SCAN_CAMERA=3> #线阵相机类型的控制串口(使用同一个串口)
)
