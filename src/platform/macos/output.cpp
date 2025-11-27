#include "output.h"

#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>

#include <cctype>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include "core/logging.h"
#include "platform/macos/event_tag.h"

namespace caps::platform::macos {

namespace {

// Lightweight helpers that convert human-friendly config tokens into the CGKeyCode
// values expected by CGEventCreateKeyboardEvent.

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

// Maps ASCII letters to CGKeyCode constants.
std::optional<CGKeyCode> LookupLetter(char letter) {
    static const std::unordered_map<char, CGKeyCode> kLetterMap = {
        {'A', kVK_ANSI_A}, {'B', kVK_ANSI_B}, {'C', kVK_ANSI_C}, {'D', kVK_ANSI_D}, {'E', kVK_ANSI_E},
        {'F', kVK_ANSI_F}, {'G', kVK_ANSI_G}, {'H', kVK_ANSI_H}, {'I', kVK_ANSI_I}, {'J', kVK_ANSI_J},
        {'K', kVK_ANSI_K}, {'L', kVK_ANSI_L}, {'M', kVK_ANSI_M}, {'N', kVK_ANSI_N}, {'O', kVK_ANSI_O},
        {'P', kVK_ANSI_P}, {'Q', kVK_ANSI_Q}, {'R', kVK_ANSI_R}, {'S', kVK_ANSI_S}, {'T', kVK_ANSI_T},
        {'U', kVK_ANSI_U}, {'V', kVK_ANSI_V}, {'W', kVK_ANSI_W}, {'X', kVK_ANSI_X}, {'Y', kVK_ANSI_Y},
        {'Z', kVK_ANSI_Z},
    };

    const auto it = kLetterMap.find(letter);
    if (it != kLetterMap.end()) {
        return it->second;
    }
    return std::nullopt;
}

// Handles named keys such as arrow/home/end plus single characters.
std::optional<CGKeyCode> LookupNamedKey(const std::string& token) {
    static const std::unordered_map<std::string, CGKeyCode> kNamedKeys = {
        {"LEFT", kVK_LeftArrow},     {"RIGHT", kVK_RightArrow}, {"UP", kVK_UpArrow},
        {"DOWN", kVK_DownArrow},     {"ESC", kVK_Escape},       {"ESCAPE", kVK_Escape},
        {"ENTER", kVK_Return},       {"RETURN", kVK_Return},    {"TAB", kVK_Tab},
        {"SPACE", kVK_Space},        {"BACKSPACE", kVK_Delete}, {"DELETE", kVK_ForwardDelete},
        {"HOME", kVK_Home},          {"END", kVK_End},          {"PAGEUP", kVK_PageUp},
        {"PAGEDOWN", kVK_PageDown},  {"F1", kVK_F1},            {"F2", kVK_F2},
        {"F3", kVK_F3},              {"F4", kVK_F4},            {"F5", kVK_F5},
        {"F6", kVK_F6},              {"F7", kVK_F7},            {"F8", kVK_F8},
        {"F9", kVK_F9},              {"F10", kVK_F10},          {"F11", kVK_F11},
        {"F12", kVK_F12},
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

// Normalizes any supported token (letters, names, or hex key codes).
std::optional<CGKeyCode> LookupKeyCode(const std::string& action) {
    const std::string normalized = NormalizeToken(action);

    if (normalized.rfind("0X", 0) == 0) {
        std::istringstream stream(normalized.substr(2));
        unsigned int value = 0;
        stream >> std::hex >> value;
        if (!stream.fail()) {
            return static_cast<CGKeyCode>(value);
        }
    }

    return LookupNamedKey(normalized);
}

} // namespace

// Converts the platform-neutral Modifiers enum to macOS CGEventFlags.
CGEventFlags ModifiersToCGFlags(caps::core::Modifiers modifiers) {
    CGEventFlags flags = 0;
    if (caps::core::HasModifier(modifiers, caps::core::Modifiers::Shift)) {
        flags |= kCGEventFlagMaskShift;
    }
    if (caps::core::HasModifier(modifiers, caps::core::Modifiers::Control)) {
        flags |= kCGEventFlagMaskControl;
    }
    if (caps::core::HasModifier(modifiers, caps::core::Modifiers::Alt)) {
        flags |= kCGEventFlagMaskAlternate;
    }
    if (caps::core::HasModifier(modifiers, caps::core::Modifiers::Meta)) {
        flags |= kCGEventFlagMaskCommand;
    }
    return flags;
}

// Emits a synthetic key press/release corresponding to the mapped action string.
void Output::Emit(const std::string& action, bool pressed, caps::core::Modifiers modifiers) {
    const auto key_code = LookupKeyCode(action);
    if (!key_code) {
        core::logging::Warn("[macOS::Output] Unknown action '" + action + "'");
        return;
    }

    CGEventRef event = CGEventCreateKeyboardEvent(nullptr, *key_code, pressed);
    if (!event) {
        core::logging::Error("[macOS::Output] Failed to create CGEvent for " + action);
        return;
    }

    // Preserve any active modifier keys (Ctrl, Shift, Alt/Option, Cmd) from the original event.
    // This allows combinations like Ctrl+mapped-key to work correctly.
    const CGEventFlags mod_flags = ModifiersToCGFlags(modifiers);
    if (mod_flags != 0) {
        CGEventSetFlags(event, mod_flags);
    }

    CGEventSetIntegerValueField(event, kCGEventSourceUserData, kSyntheticEventTag);
    // Fire the event at the HID tap so it behaves like real hardware input.
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
}

} // namespace caps::platform::macos
