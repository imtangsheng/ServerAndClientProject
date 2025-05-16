
# 设置可执行文件名称
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui SerialPort LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui SerialPort LinguistTools)

# 将源文件组织在变量中
set(CAMERA_PUBLIC_FILE
    "${INCLUDE_DIR}interface/iPluginDevice.h"
    #"${INCLUDE_DIR}interface/icamera.h"
    CameraPlugin.h CameraPlugin.cpp
    CameraPlugin.json
)
# 定义一个函数来创建相机插件
function(plugin_add_ PLUGIN_NAME PLUGIN_SOURCE)
    add_library(${PLUGIN_NAME} SHARED 
        ${PLUGIN_SOURCE}
        ${ARGN}  # 接收额外的源文件参数
        "${PLUGIN_NAME}.qrc"
     )

    qt_add_resources(${PLUGIN_NAME} "${PLUGIN_NAME}.qrc")

    target_link_libraries(${PLUGIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::SerialPort
        ${SHARE_NAME}
    )

    # add_dependencies(${PLUGIN_NAME} ${SHARE_NAME})
endfunction()
#INTERFACE
set(HiKvision_FILE
    MVS/hikvision_camera.h MVS/hikvision_camera.cpp
    "${INCLUDE_DIR}interface/iSerialPort.h"
    ${PROJECT_SOURCE_DIR}/public/serial/ActiveSerial.h ${PROJECT_SOURCE_DIR}/public/serial/ActiveSerial.cpp
)

set(DvpLineScan_FILE
    "LineScanCamera/readme.md"
    LineScanCamera/dvp_line_scan_camera.h LineScanCamera/dvp_line_scan_camera.cpp
    ${PROJECT_SOURCE_DIR}/public/serial/LeiShenLidarN10Plus.h ${PROJECT_SOURCE_DIR}/public/serial/LeiShenLidarN10Plus.cpp
)

plugin_add_(HiKvisionCamera ${CAMERA_PUBLIC_FILE} ${HiKvision_FILE})
plugin_add_(DvpLineScanCamera ${CAMERA_PUBLIC_FILE} ${DvpLineScan_FILE})

#自动生成翻译文本
#qt_add_lupdate(${PLUGIN_NAME}
#    TS_FILES ${PROJECT_SOURCE_DIR}/assets/translations/${PLUGIN_NAME}_zh_CN.ts
#    OPTIONS
#    -no-obsolete        # 删除过时的翻译
#)


target_link_libraries(HiKvisionCamera PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/MVS/MvCameraControl.lib"
)
target_link_libraries(DvpLineScanCamera PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/LineScanCamera/DVPCamera64.lib"
)

#设置编译编译定义是在编译过程中传递给预处理器的符号，可以用来控制代码中的条件编译。
target_compile_definitions(HiKvisionCamera PRIVATE
    DEVICE_TYPE_CAMERA_HiKvision
    DEVICE_TYPE_CAMERA=2#三叶草相机设备类型的控制串口
)
target_compile_definitions(DvpLineScanCamera PRIVATE
    DEVICE_TYPE_CAMERA_DvpLineScan
    #DVP_LIB_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Do3think/DVPCamera64.lib\"
)
