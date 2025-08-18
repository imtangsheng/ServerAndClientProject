@echo off
chcp 65001 >nul

echo.
echo ==========================================
echo        配置 Nginx 开机自启动
echo ==========================================
echo.

:: 检查管理员权限
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo [错误] 此操作需要管理员权限
    echo 请右键点击此脚本，选择"以管理员身份运行"
    echo.
    pause
    exit /b 1
)

:: 获取当前脚本所在目录
set SCRIPT_DIR=%~dp0
set START_SCRIPT=%SCRIPT_DIR%nginx_start.bat

:: 检查启动脚本是否存在
if not exist "%START_SCRIPT%" (
    echo [错误] 未找到启动脚本: %START_SCRIPT%
    echo.
    pause
    exit /b 1
)

echo [信息] 正在创建定时任务...

:: 删除已存在的任务（如果有）
schtasks /delete /tn "Nginx Auto Start" /f >nul 2>&1

:: 创建新的定时任务 "SYSTEM" 是系统账户 "%USERNAME%" 适用于需要在特定用户登录后才运行的
schtasks /create /tn "Nginx Auto Start" /tr "\"%START_SCRIPT%\" auto" /sc onstart /ru "SYSTEM" /rl highest /f

if %errorlevel% equ 0 (
    echo [成功] 开机自启动配置完成！
    echo.
    echo 任务名称: Nginx Auto Start
    echo 触发条件: 系统启动时
    echo 执行脚本: %START_SCRIPT%
    echo.
    echo 您可以在"任务计划程序"中查看和管理此任务
) else (
    echo [错误] 配置开机自启动失败
    echo.
)

echo.
pause