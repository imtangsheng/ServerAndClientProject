
# 设置可执行文件名称
#set(PLUGIN_NAME HikvisionCamera)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui LinguistTools)

#add_subdirectory(Do2think)#度申线阵相机开发包
#add_subdirectory(MVS)#海康相机开发包

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
        South
    )

    add_dependencies(${PLUGIN_NAME} South)
endfunction()
#INTERFACE
set(HiKvision_FILE
    MVS/hikvision_camera.h MVS/hikvision_camera.cpp
)

set(DvpLineScan_FILE
    "Do3think/readme.md"
    Do3think/dvp_line_scan_camera.h Do3think/dvp_line_scan_camera.cpp
)

plugin_add_(HikvisionCamera ${CAMERA_PUBLIC_FILE} ${HiKvision_FILE})
plugin_add_(DvpLineScanCamera ${CAMERA_PUBLIC_FILE} ${DvpLineScan_FILE})

#自动生成翻译文本
#qt_add_lupdate(${PLUGIN_NAME}
#    TS_FILES ${PROJECT_SOURCE_DIR}/assets/translations/${PLUGIN_NAME}_zh_CN.ts
#    OPTIONS
#    -no-obsolete        # 删除过时的翻译
#)


target_link_libraries(HikvisionCamera PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/MVS/MvCameraControl.lib"
)
target_link_libraries(DvpLineScanCamera PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Do3think/DVPCamera64.lib"
)

#设置编译编译定义是在编译过程中传递给预处理器的符号，可以用来控制代码中的条件编译。
target_compile_definitions(HikvisionCamera PRIVATE
    DEVICE_TYPE_CAMERA_HiKvision
)
target_compile_definitions(DvpLineScanCamera PRIVATE
    DEVICE_TYPE_CAMERA_DvpLineScan
    #DVP_LIB_PATH=\"${CMAKE_CURRENT_SOURCE_DIR}/Do3think/DVPCamera64.lib\"
)
