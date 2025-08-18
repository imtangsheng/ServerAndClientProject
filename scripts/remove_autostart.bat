@echo off
chcp 65001 >nul

echo [信息] 正在移除nginx开机自启动...

schtasks /delete /tn "Nginx Auto Start" /f

if %errorlevel% equ 0 (
    echo [成功] 开机自启动已移除
) else (
    echo [错误] 移除失败或任务不存在
)

echo.
pause