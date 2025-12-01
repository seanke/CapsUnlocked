#include "output.h"

#include <windows.h>

#include <cctype>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

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
        {"SHIFT", VK_SHIFT},       {"LSHIFT", VK_LSHIFT},
        {"RSHIFT", VK_RSHIFT},     {"CTRL", VK_CONTROL},
        {"CONTROL", VK_CONTROL},   {"ALT", VK_MENU},
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

struct ActionToken {
    std::string key;
    bool hold{false};
};

std::vector<ActionToken> SplitTokens(const std::string& action) {
    std::istringstream stream(action);
    std::vector<ActionToken> tokens;
    std::string token;
    while (stream >> token) {
        ActionToken parsed;
        parsed.hold = !token.empty() && token.back() == '!';
        if (parsed.hold) {
            token.pop_back();
        }
        parsed.key = token;
        tokens.push_back(std::move(parsed));
    }
    return tokens;
}

bool SendSingle(WORD vk_code, bool pressed) {
    INPUT input = {};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = vk_code;
    input.ki.dwFlags = pressed ? 0 : KEYEVENTF_KEYUP;
    input.ki.dwExtraInfo = kSyntheticEventTag;

    const UINT result = SendInput(1, &input, sizeof(INPUT));
    if (result != 1) {
        const DWORD error = GetLastError();
        std::ostringstream msg;
        msg << "[Windows::Output] Failed to send input for vk=0x" << std::hex << vk_code
            << " (pressed=" << pressed << "), error code: 0x" << std::hex << error;
        core::logging::Error(msg.str());
        return false;
    }
    return true;
}

} // namespace

// Emits a synthetic key press/release corresponding to the mapped action string
void Output::Emit(const std::string& action, bool pressed) {
    const auto tokens = SplitTokens(action);
    if (tokens.empty()) {
        core::logging::Warn("[Windows::Output] Empty action");
        return;
    }

    std::vector<std::pair<WORD, bool>> sequence;
    sequence.reserve(tokens.size() * 2);

    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& tok = tokens[i];
        const auto vk_code = LookupKeyCode(tok.key);
        if (!vk_code) {
            core::logging::Warn("[Windows::Output] Unknown action token '" + tok.key + "' in '" + action + "'");
            return;
        }

        if (tok.hold) {
            if (i + 1 >= tokens.size()) {
                core::logging::Warn("[Windows::Output] Hold token '" + tok.key + "!' has no following key in '" + action + "'");
                return;
            }
            const auto next_code = LookupKeyCode(tokens[i + 1].key);
            if (!next_code) {
                core::logging::Warn("[Windows::Output] Unknown action token '" + tokens[i + 1].key + "' in '" + action + "'");
                return;
            }
            sequence.emplace_back(*vk_code, true);    // hold down
            sequence.emplace_back(*next_code, true);  // tap target
            sequence.emplace_back(*next_code, false); // release target
            sequence.emplace_back(*vk_code, false);   // release hold
            ++i; // consume the next token
        } else {
            sequence.emplace_back(*vk_code, true);
            sequence.emplace_back(*vk_code, false);
        }
    }

    if (pressed) {
        for (const auto& [code, down] : sequence) {
            if (!SendSingle(code, down)) {
                return;
            }
        }
    }
    // Macro completes on press; releases are ignored for synthetic sequences.
}

} // namespace caps::platform::windows
