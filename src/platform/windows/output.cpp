#include "output.h"

#include <windows.h>

#include <cctype>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include "core/logging.h"
#include "platform/windows/keyboard_hook.h"

namespace caps::platform::windows {

namespace {

// Lightweight helpers that convert human-friendly config tokens into the VK_* codes
// expected by SendInput.

std::string NormalizeToken(const std::string& token) {
    std::string normalized;
    normalized.reserve(token.size());
    for (char ch : token) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    return normalized;
}

// Maps ASCII letters to VK_* constants
std::optional<WORD> LookupLetter(char letter) {
    // For letters A-Z, VK codes match ASCII
    if (letter >= 'A' && letter <= 'Z') {
        return static_cast<WORD>(letter);
    }
    return std::nullopt;
}

// Handles named keys such as arrow/home/end plus single characters
std::optional<WORD> LookupNamedKey(const std::string& token) {
    static const std::unordered_map<std::string, WORD> kNamedKeys = {
        {"LEFT", VK_LEFT},         {"RIGHT", VK_RIGHT},
        {"UP", VK_UP},             {"DOWN", VK_DOWN},
        {"ESC", VK_ESCAPE},        {"ESCAPE", VK_ESCAPE},
        {"ENTER", VK_RETURN},      {"RETURN", VK_RETURN},
        {"TAB", VK_TAB},           {"SPACE", VK_SPACE},
        {"BACKSPACE", VK_BACK},    {"DELETE", VK_DELETE},
        {"HOME", VK_HOME},         {"END", VK_END},
        {"PAGEUP", VK_PRIOR},      {"PAGEDOWN", VK_NEXT},
        {"F1", VK_F1},             {"F2", VK_F2},
        {"F3", VK_F3},             {"F4", VK_F4},
        {"F5", VK_F5},             {"F6", VK_F6},
        {"F7", VK_F7},             {"F8", VK_F8},
        {"F9", VK_F9},             {"F10", VK_F10},
        {"F11", VK_F11},           {"F12", VK_F12},
    };

    if (token.size() == 1 && std::isalpha(static_cast<unsigned char>(token.front()))) {
        return LookupLetter(token.front());
    }

    const auto it = kNamedKeys.find(token);
    if (it != kNamedKeys.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Normalizes any supported token (letters, names, or hex key codes)
std::optional<WORD> LookupKeyCode(const std::string& action) {
    const std::string normalized = NormalizeToken(action);

    if (normalized.rfind("0X", 0) == 0) {
        std::istringstream stream(normalized.substr(2));
        unsigned int value = 0;
        stream >> std::hex >> value;
        if (!stream.fail()) {
            return static_cast<WORD>(value);
        }
    }

    return LookupNamedKey(normalized);
}

} // namespace

// Emits a synthetic key press/release corresponding to the mapped action string
void Output::Emit(const std::string& action, bool pressed) {
    const auto vk_code = LookupKeyCode(action);
    if (!vk_code) {
        core::logging::Warn("[Windows::Output] Unknown action '" + action + "'");
        return;
    }

    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = *vk_code;
    input.ki.dwFlags = pressed ? 0 : KEYEVENTF_KEYUP;
    input.ki.dwExtraInfo = kSyntheticEventTag;

    const UINT result = SendInput(1, &input, sizeof(INPUT));
    if (result != 1) {
        const DWORD error = GetLastError();
        std::ostringstream msg;
        msg << "[Windows::Output] Failed to send input for " << action 
            << ", error code: 0x" << std::hex << error;
        core::logging::Error(msg.str());
    }
}

} // namespace caps::platform::windows
