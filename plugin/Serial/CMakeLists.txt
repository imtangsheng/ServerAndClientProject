
# 设置可执行文件名称
set(PLUGIN_NAME Serial)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core SerialPort)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core SerialPort)

# 将源文件组织在变量中
set(Serial_SOURCES
    "${INCLUDE_DIR}interface/iSerialPort.h"
    ActiveSerial.h ActiveSerial.cpp
    "${INCLUDE_DIR}interface/iPluginDevice.h"
    SerialPlugin.h SerialPlugin.cpp
    SerialPlugin.json "readme.md"
)

#INTERFACE
add_library(${PLUGIN_NAME} SHARED 
    ${Serial_SOURCES}
    "${PLUGIN_NAME}.qrc"
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
    DEVICE_TYPE_CAR #小车中控箱设备控制的串口控制板
    DEVICE_TYPE_CLOVER #三叶草相机设备类型的控制串口
)
