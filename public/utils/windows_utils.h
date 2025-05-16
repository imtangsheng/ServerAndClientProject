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

// 设置开机自启
//#include <QCoreApplication>
inline void SetAutoStart(bool enable) {
    QString appName = QCoreApplication::applicationName();
    //QString appPath = QApplication::applicationFilePath().replace("/", "\\");
        // 获取可执行文件路径（适用于QCoreApplication）
    QString appPath = QCoreApplication::arguments().first();
    appPath = QDir::toNativeSeparators(appPath);
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    if (enable) {
        reg.setValue(appName, appPath);
    } else {
        reg.remove(appName);
    }
}

inline bool IsAutoStartEnabled() {
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",QSettings::NativeFormat);
    return reg.contains(QCoreApplication::applicationName());
}

// 获取最后的系统错误码
inline QString GetSysError() {
    DWORD errorCode = GetLastError();
    LPVOID lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL
    );
    QString sysError = QString::fromWCharArray((LPTSTR)lpMsgBuf);
    LocalFree(lpMsgBuf);
    return sysError;
}

#include <shellapi.h>
//移动指定目录到回收站
inline bool MoveToRecycleBin(const QString& path) {
    if (path.isEmpty()) {
        return false;
    }
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
        return false;
    }
    WCHAR from[MAX_PATH];
    path.toWCharArray(from);
    from[path.length()] = '\0';

    SHFILEOPSTRUCTW fileOp;
    memset(&fileOp, 0, sizeof(fileOp));
    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = from;
    fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

    int ret = SHFileOperationW(&fileOp);
    return (ret == 0);
    return false;
}
#endif // WINDOWS_UTILS_H
