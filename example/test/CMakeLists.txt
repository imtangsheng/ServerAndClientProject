# 设置可执行文件名称
set(APP_NAME ProjectTest)
# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

# 创建可执行文件
add_executable(${APP_NAME}
    main.cpp
    ${INCLUDE_DIR}common/FileReadAndSave.ixx
   "TCase.h" "TCase.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  #set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 20)
endif()

# 链接 Qt 库
target_link_libraries(${APP_NAME} PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    ${SHARE_NAME}
)
