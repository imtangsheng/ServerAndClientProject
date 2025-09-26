在 Debug、Release 中构建项目的先决条件,使用CMake管理构建

Install Qt 6
==============================================================================

* Download the installer at https://www.qt.io/download-qt-installer
* Install version 6.9.0 and the corresponding binaries for your developpement environment (i.e. MSVC 2022 64-bit)
* /!\ Qt 5 is not supported.
* source code and optional Qt modules are not needed (it can save a lot of space)
* Allow for executing binaries by adding the current path to your "PATH" environment variable: <qt_install_dir>6.9.0\msvc2019_64\bin\

Configure Visual Studio
==============================================================================

* Open Working Directory -> CMakeLists.txt:


Install Qt VS Tool:
==============================================================================

It is a plugin needed to compile the Qt dependencies (moc, ui, qrc).

You can find it in Tools->Extensions and Updates->Qt Visual Stutio Tools.

Add the path where Qt is installed on your machine in:

* Qt VS Tools->Qt Options->Qt Versions->Add (i.e. `C:\Qt\6.x.x\msvc2022_64`)

  
Build with Inno Setup:
==============================================================================
* Download and install the last version of Inno Setup : https://jrsoftware.org/isdl.php

Others:
==============================================================================
* [命名约定](./docs/项目命名约定.md)
* Server项目路径 ->main->server
* Client项目路径 ->main->client