
# 查找所需的 Qt 组件
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)

add_executable(TestEXE main.cpp)

# 链接 Qt 库
target_link_libraries(TestEXE PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
)