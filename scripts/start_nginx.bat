@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo.
echo ==========================================
echo           Nginx 服务启动程序
echo ==========================================
echo.

:: 获取当前脚本所在目录（去掉末尾的反斜杠）
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

:: 设置nginx路径为脚本所在目录
set NGINX_PATH=%SCRIPT_DIR%
set NGINX_EXE=%NGINX_PATH%\nginx.exe
:: 各种路径变量的区别：
echo 脚本完整路径: %0
echo 脚本所在目录(含反斜杠): %~dp0  
echo 脚本文件名: %~nx0
echo 当前工作目录: %cd%

echo [调试] 脚本目录: %SCRIPT_DIR%
echo [调试] 当前工作目录: %cd%
echo [调试] nginx路径: %NGINX_PATH%
echo [调试] nginx程序: %NGINX_EXE%
echo.

:: 检查nginx是否存在
if not exist "%NGINX_EXE%" (
    echo [错误] 未找到nginx程序: %NGINX_EXE%
    echo 请确保nginx.exe与此脚本在同一目录下
    echo.
    echo 当前查找路径: %NGINX_PATH%
    echo 脚本所在目录: %SCRIPT_DIR%
    echo 当前工作目录: %cd%
    echo.
    pause
    exit /b 1
)


:: 检查nginx是否已经运行
tasklist /FI "IMAGENAME eq nginx.exe" 2>NUL | find /I /N "nginx.exe" >NUL
if "%ERRORLEVEL%"=="0" (
    echo [信息] nginx已经在运行中
    echo.
    goto :show_status
)

:: 切换到nginx目录（重要！）
echo [信息] 切换到nginx目录: %NGINX_PATH%
cd /d "%NGINX_PATH%"

:: 启动nginx
echo [信息] 正在启动nginx服务...
start "" "%NGINX_EXE%"

:: 等待一下让nginx完全启动
timeout /t 2 /nobreak >nul

:: 检查启动是否成功
tasklist /FI "IMAGENAME eq nginx.exe" 2>NUL | find /I /N "nginx.exe" >NUL
if "%ERRORLEVEL%"=="0" (
    echo [成功] nginx服务启动成功！
    echo.
) else (
    echo [错误] nginx服务启动失败！
    echo 请检查nginx配置文件是否正确
    echo 当前nginx目录: %NGINX_PATH%
    echo.
    
    :: 显示可能的错误信息
    if exist "%NGINX_PATH%\logs\error.log" (
        echo 最近的错误日志：
        echo ----------------------------------------
        powershell -command "Get-Content '%NGINX_PATH%\logs\error.log' -Tail 5"
        echo ----------------------------------------
    )
    
    pause
    exit /b 1
)

:show_status
echo 当前nginx进程信息：
tasklist /FI "IMAGENAME eq nginx.exe" /FO TABLE
echo.
echo nginx服务已成功运行
echo 当前nginx工作目录: %NGINX_PATH%

:: 显示nginx配置的端口信息
if exist "%NGINX_PATH%\conf\nginx.conf" (
    echo.
    echo 检测到的监听端口：
    findstr /i "listen" "%NGINX_PATH%\conf\nginx.conf" | findstr /v "#"
)

echo.
echo 可以访问 http://localhost 来测试
echo.

:: 如果是开机自启动，5秒后自动关闭
if "%1"=="auto" (
    echo 5秒后自动关闭...
    timeout /t 5 /nobreak >nul
    exit /b 0
)

pause