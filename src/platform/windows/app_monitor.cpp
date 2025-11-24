#include "app_monitor.h"

#include <windows.h>
#include <psapi.h>
#include <string>

namespace caps::platform::windows {

namespace {

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return "";
    }
    
    // Use -1 to auto-detect null terminator for safer conversion
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, 
                                          nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) {
        return "";
    }
    
    // size_needed includes the null terminator, so subtract 1 for the string content
    std::string result(size_needed - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1,
                       &result[0], size_needed, nullptr, nullptr);
    return result;
}

std::string GetProcessName(HWND hwnd) {
    DWORD process_id = 0;
    GetWindowThreadProcessId(hwnd, &process_id);
    if (process_id == 0) {
        return "";
    }

    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, process_id);
    if (!process) {
        return "";
    }

    wchar_t exe_path[MAX_PATH] = {};
    DWORD size = MAX_PATH;
    if (QueryFullProcessImageNameW(process, 0, exe_path, &size)) {
        // Extract just the executable name from the full path
        std::wstring wide_path(exe_path);
        const size_t last_slash = wide_path.find_last_of(L"\\/");
        if (last_slash != std::wstring::npos && last_slash + 1 < wide_path.size()) {
            std::wstring exe_name = wide_path.substr(last_slash + 1);
            // Remove .exe extension if present
            const size_t ext_pos = exe_name.rfind(L".exe");
            if (ext_pos != std::wstring::npos && ext_pos == exe_name.size() - 4) {
                exe_name = exe_name.substr(0, ext_pos);
            }
            // Convert wide string to UTF-8
            std::string result = WideToUtf8(exe_name);
            CloseHandle(process);
            return result;
        }
    }

    CloseHandle(process);
    return "";
}

} // namespace

std::string AppMonitor::CurrentAppName() const {
    HWND foreground = GetForegroundWindow();
    if (!foreground) {
        return "";
    }

    // Try to get the process name
    std::string process_name = GetProcessName(foreground);
    if (!process_name.empty()) {
        return process_name;
    }

    // Fallback to window title if process name unavailable
    wchar_t title[256] = {};
    if (GetWindowTextW(foreground, title, 256) > 0) {
        return WideToUtf8(std::wstring(title));
    }

    return "";
}

} // namespace caps::platform::windows
