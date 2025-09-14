#include "util.h"
#include <shlwapi.h>
#include <cwctype>

std::wstring get_exe_dir() {
    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    PathRemoveFileSpecW(path);
    return std::wstring(path);
}

void trim(std::wstring& s) {
    size_t start = s.find_first_not_of(L" \t\r\n");
    size_t end   = s.find_last_not_of(L" \t\r\n");
    if (start == std::wstring::npos) { s.clear(); return; }
    s = s.substr(start, end - start + 1);
}

UINT token_to_vk(const std::wstring& token) {
    if (token.size() == 1) {
        wchar_t c = token[0];
        if (c >= L'a' && c <= L'z') c = static_cast<wchar_t>(std::towupper(c));
        return static_cast<UINT>(static_cast<unsigned short>(c));
    }
    if (_wcsicmp(token.c_str(), L"Left") == 0) return VK_LEFT;
    if (_wcsicmp(token.c_str(), L"Right") == 0) return VK_RIGHT;
    if (_wcsicmp(token.c_str(), L"Up") == 0) return VK_UP;
    if (_wcsicmp(token.c_str(), L"Down") == 0) return VK_DOWN;
    if (_wcsicmp(token.c_str(), L"Enter") == 0) return VK_RETURN;
    if (_wcsicmp(token.c_str(), L"Esc") == 0 || _wcsicmp(token.c_str(), L"Escape") == 0) return VK_ESCAPE;
    if (_wcsicmp(token.c_str(), L"Tab") == 0) return VK_TAB;
    if (_wcsicmp(token.c_str(), L"Space") == 0 || _wcsicmp(token.c_str(), L"Spacebar") == 0) return VK_SPACE;
    if (_wcsicmp(token.c_str(), L"Backspace") == 0) return VK_BACK;
    if (_wcsicmp(token.c_str(), L"Delete") == 0) return VK_DELETE;
    if (_wcsicmp(token.c_str(), L"Home") == 0) return VK_HOME;
    if (_wcsicmp(token.c_str(), L"End") == 0) return VK_END;
    if (_wcsicmp(token.c_str(), L"PageUp") == 0) return VK_PRIOR;
    if (_wcsicmp(token.c_str(), L"PageDown") == 0) return VK_NEXT;

    if (token.rfind(L"0x", 0) == 0 || token.rfind(L"0X", 0) == 0) {
        unsigned int v = 0;
        if (swscanf(token.c_str(), L"%x", &v) == 1) return static_cast<UINT>(v);
    }
    return 0;
}

