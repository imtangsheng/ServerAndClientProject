@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo.
echo ==========================================
echo           Nginx 服务停止程序
echo ==========================================
echo.

:: 获取当前脚本所在目录（去掉末尾的反斜杠）
set SCRIPT_DIR=%~dp0
if "%SCRIPT_DIR:~-1%"=="\" set SCRIPT_DIR=%SCRIPT_DIR:~0,-1%

:: 设置nginx路径为脚本所在目录
set NGINX_PATH=%SCRIPT_DIR%
set NGINX_EXE=%NGINX_PATH%\nginx.exe

:: 检查nginx是否运行
tasklist /FI "IMAGENAME eq nginx.exe" 2>NUL | find /I /N "nginx.exe" >NUL
if not "%ERRORLEVEL%"=="0" (
    echo [信息] nginx服务未运行
    echo.
    pause
    exit /b 0
)

echo [信息] 正在停止nginx服务...

:: 优雅停止nginx
cd /d "%NGINX_PATH%"
"%NGINX_EXE%" -s quit

:: 等待进程结束
timeout /t 3 /nobreak >nul

:: 检查是否还有nginx进程
tasklist /FI "IMAGENAME eq nginx.exe" 2>NUL | find /I /N "nginx.exe" >NUL
if "%ERRORLEVEL%"=="0" (
    echo [警告] nginx进程仍在运行，强制终止...
    taskkill /F /IM nginx.exe >nul 2>&1
    if "!ERRORLEVEL!"=="0" (
        echo [成功] nginx服务已强制停止
    ) else (
        echo [错误] 无法停止nginx服务
    )
) else (
    echo [成功] nginx服务已停止
)

echo.
pause