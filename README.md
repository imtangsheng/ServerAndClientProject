# 采集软件开发

Develop: Qt6 C++ 

# 适配多个Qt5的版本
1.使用vs2022
1)打开 VS2022
2)进入菜单：扩展 -> Qt VS Tools -> Qt Versions
3)在打开的窗口中添加不同的Qt版本路径
4)选择默认的qt版本
5)清理删除Cmake缓存并重新构建项目(CMake预设文件(CMakePresets.json)的配置会更新,QTDIR：Qt安装路径)
6)重新设置版本从4)开始,默认版本 并删除CMake 缓存

