#include "util.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <system_error>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <mach-o/dyld.h>
#endif

namespace {

std::string to_upper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return s;
}

#if defined(__APPLE__)
KeyCode letter_keycode(char upper) {
    static const KeyCode letter_codes[26] = {
        kVK_ANSI_A, kVK_ANSI_B, kVK_ANSI_C, kVK_ANSI_D, kVK_ANSI_E, kVK_ANSI_F, kVK_ANSI_G,
        kVK_ANSI_H, kVK_ANSI_I, kVK_ANSI_J, kVK_ANSI_K, kVK_ANSI_L, kVK_ANSI_M, kVK_ANSI_N,
        kVK_ANSI_O, kVK_ANSI_P, kVK_ANSI_Q, kVK_ANSI_R, kVK_ANSI_S, kVK_ANSI_T, kVK_ANSI_U,
        kVK_ANSI_V, kVK_ANSI_W, kVK_ANSI_X, kVK_ANSI_Y, kVK_ANSI_Z
    };
    if (upper >= 'A' && upper <= 'Z') {
        return letter_codes[upper - 'A'];
    }
    return 0;
}

KeyCode digit_keycode(char digit) {
    static const KeyCode digit_codes[10] = {
        kVK_ANSI_0, kVK_ANSI_1, kVK_ANSI_2, kVK_ANSI_3, kVK_ANSI_4,
        kVK_ANSI_5, kVK_ANSI_6, kVK_ANSI_7, kVK_ANSI_8, kVK_ANSI_9
    };
    if (digit >= '0' && digit <= '9') {
        return digit_codes[digit - '0'];
    }
    return 0;
}
#endif

} // namespace

std::filesystem::path get_exe_dir() {
#if defined(_WIN32)
    wchar_t buffer[MAX_PATH] = {0};
    const DWORD len = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return std::filesystem::current_path();
    }
    std::filesystem::path exe_path(buffer);
#else
    uint32_t size = 0;
    if (_NSGetExecutablePath(nullptr, &size) != 0) {
        return std::filesystem::current_path();
    }
    std::vector<char> path_buf(size);
    if (_NSGetExecutablePath(path_buf.data(), &size) != 0) {
        return std::filesystem::current_path();
    }
    std::filesystem::path exe_path(path_buf.data());
#endif
    std::error_code ec;
    std::filesystem::path resolved = std::filesystem::weakly_canonical(exe_path, ec);
    if (ec) resolved = exe_path;
    resolved.remove_filename();
    return resolved;
}

void trim(std::string& s) {
    const auto not_space = [](unsigned char c) { return !std::isspace(c); };
    const auto start = std::find_if(s.begin(), s.end(), not_space);
    const auto end = std::find_if(s.rbegin(), s.rend(), not_space).base();
    if (start >= end) {
        s.clear();
        return;
    }
    s.assign(start, end);
}

KeyCode token_to_keycode(const std::string& token) {
    if (token.empty()) return 0;

    if (token.size() == 1) {
        const unsigned char ch = static_cast<unsigned char>(token[0]);
#if defined(_WIN32)
        if (std::islower(ch)) {
            return static_cast<KeyCode>(std::toupper(ch));
        }
        return static_cast<KeyCode>(ch);
#else
        if (std::isalpha(ch)) {
            return letter_keycode(static_cast<char>(std::toupper(ch)));
        }
        if (std::isdigit(ch)) {
            return digit_keycode(static_cast<char>(ch));
        }
        switch (ch) {
            case ' ': return kVK_Space;
            case '\t': return kVK_Tab;
            default: break;
        }
        return 0;
#endif
    }

    if (token.size() > 2 && (token[0] == '0') && (token[1] == 'x' || token[1] == 'X')) {
        char* end = nullptr;
        const unsigned long value = std::strtoul(token.c_str(), &end, 16);
        if (end && *end == '\0') {
            return static_cast<KeyCode>(value);
        }
    }

    const std::string upper = to_upper(token);

#if defined(_WIN32)
    if (upper == "LEFT") return VK_LEFT;
    if (upper == "RIGHT") return VK_RIGHT;
    if (upper == "UP") return VK_UP;
    if (upper == "DOWN") return VK_DOWN;
    if (upper == "ENTER" || upper == "RETURN") return VK_RETURN;
    if (upper == "ESC" || upper == "ESCAPE") return VK_ESCAPE;
    if (upper == "TAB") return VK_TAB;
    if (upper == "SPACE" || upper == "SPACEBAR") return VK_SPACE;
    if (upper == "BACKSPACE") return VK_BACK;
    if (upper == "DELETE" || upper == "FORWARDDELETE") return VK_DELETE;
    if (upper == "HOME") return VK_HOME;
    if (upper == "END") return VK_END;
    if (upper == "PAGEUP") return VK_PRIOR;
    if (upper == "PAGEDOWN") return VK_NEXT;
#else
    if (upper == "LEFT") return kVK_LeftArrow;
    if (upper == "RIGHT") return kVK_RightArrow;
    if (upper == "UP") return kVK_UpArrow;
    if (upper == "DOWN") return kVK_DownArrow;
    if (upper == "ENTER" || upper == "RETURN") return kVK_Return;
    if (upper == "ESC" || upper == "ESCAPE") return kVK_Escape;
    if (upper == "TAB") return kVK_Tab;
    if (upper == "SPACE" || upper == "SPACEBAR") return kVK_Space;
    if (upper == "BACKSPACE" || upper == "DELETE") return kVK_Delete;
    if (upper == "FORWARDDELETE") return kVK_ForwardDelete;
    if (upper == "HOME") return kVK_Home;
    if (upper == "END") return kVK_End;
    if (upper == "PAGEUP") return kVK_PageUp;
    if (upper == "PAGEDOWN") return kVK_PageDown;
#endif

    return 0;
}
