#include "config.h"
#include "util.h"
#include <cwchar>

static void ensure_default_config(const std::wstring& path) {
    FILE* f = _wfopen(path.c_str(), L"r, ccs=UTF-8");
    if (f) { fclose(f); return; }
    f = _wfopen(path.c_str(), L"w, ccs=UTF-8");
    if (!f) return;
    fwprintf(f, L"# CapsUnlocked config\n");
    fwprintf(f, L"# Map source=target using virtual keys or names.\n");
    fwprintf(f, L"# Hold CapsLock to activate.\n\n");
    fwprintf(f, L"h=Left\n");
    fwprintf(f, L"j=Down\n");
    fwprintf(f, L"k=Up\n");
    fwprintf(f, L"l=Right\n");
    fclose(f);
}

KeyMap load_config() {
    std::wstring dir = get_exe_dir();
    std::wstring cfgPath = dir + L"\\capsunlocked.ini";
    ensure_default_config(cfgPath);

    KeyMap map;
    FILE* f = _wfopen(cfgPath.c_str(), L"r, ccs=UTF-8");
    if (!f) {
        map[L'H'] = VK_LEFT;
        map[L'J'] = VK_DOWN;
        map[L'K'] = VK_UP;
        map[L'L'] = VK_RIGHT;
        return map;
    }

    wchar_t buf[512];
    while (fgetws(buf, static_cast<int>(sizeof(buf)/sizeof(buf[0])), f)) {
        std::wstring line(buf);
        trim(line);
        if (line.empty()) continue;
        if (line[0] == L'#' || line[0] == L';') continue;
        size_t eq = line.find(L'=');
        if (eq == std::wstring::npos) continue;
        std::wstring left = line.substr(0, eq);
        std::wstring right = line.substr(eq + 1);
        trim(left); trim(right);
        if (left.empty() || right.empty()) continue;
        UINT src = token_to_vk(left);
        UINT dst = token_to_vk(right);
        if (src != 0 && dst != 0) {
            map[src] = dst;
        }
    }
    fclose(f);

    if (map.empty()) {
        map[L'H'] = VK_LEFT;
        map[L'J'] = VK_DOWN;
        map[L'K'] = VK_UP;
        map[L'L'] = VK_RIGHT;
    }
    return map;
}

