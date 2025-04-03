#ifndef WINDOWS_UTILS_H
#define WINDOWS_UTILS_H

#include <windows.h>
#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

// 读取系统主题设置
inline bool IsSystemDarkMode() {
    HKEY hKey;
    DWORD value = 1; // 默认浅色模式
    DWORD size = sizeof(DWORD);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegQueryValueEx(hKey, L"AppsUseLightTheme", nullptr, nullptr, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    return value == 0; // 0 表示深色模式，1 表示浅色模式
}

inline void SetDarkMode(HWND hwnd, BOOL darkMode = TRUE) {
    // BOOL darkMode = TRUE;
    HRESULT hr = DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
    if (FAILED(hr)) {
        TCHAR errorMsg[256];
        wsprintf(errorMsg, TEXT("设置主题深色模式失败，错误代码: %ld"), hr);
        MessageBox(NULL, errorMsg, TEXT("错误"), MB_OK | MB_ICONERROR);
    }
}

#endif // WINDOWS_UTILS_H
