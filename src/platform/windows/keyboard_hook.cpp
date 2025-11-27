#include "keyboard_hook.h"

// CapsUnlocked Windows adapter: low-level keyboard hook bridge that feeds
// key events into the shared layer controller.

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

#include "core/layer/layer_controller.h"
#include "core/logging.h"

namespace caps::platform::windows {

// Static instance pointer for the global hook callback
KeyboardHook* KeyboardHook::instance_ = nullptr;

KeyboardHook::KeyboardHook(AppMonitor* app_monitor) : app_monitor_(app_monitor) {}

void KeyboardHook::Install(core::LayerController& controller) {
    controller_ = &controller;
    instance_ = this;
    core::logging::Info("[Windows::KeyboardHook] Installing low-level keyboard hook");
    
    // Install WH_KEYBOARD_LL hook for low-level keyboard interception
    hook_handle_ = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, nullptr, 0);
    if (!hook_handle_) {
        const DWORD error = GetLastError();
        std::ostringstream msg;
        msg << "[Windows::KeyboardHook] Failed to install keyboard hook, error code: 0x" 
            << std::hex << error;
        core::logging::Error(msg.str());
    } else {
        core::logging::Info("[Windows::KeyboardHook] Keyboard hook installed successfully");
    }
}

void KeyboardHook::StartListening() {
    core::logging::Info("[Windows::KeyboardHook] Starting to listen for keyboard events");
    // Hook is already active after Install; no additional action needed here.
    // The message loop in PlatformApp::Run() will process keyboard events.
}

void KeyboardHook::StopListening() {
    core::logging::Info("[Windows::KeyboardHook] Stopping keyboard listening");
    if (hook_handle_) {
        UnhookWindowsHookEx(hook_handle_);
        hook_handle_ = nullptr;
    }
    instance_ = nullptr;
}

// Static callback for low-level keyboard events
LRESULT CALLBACK KeyboardHook::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (instance_) {
        return instance_->HandleKeyboardEvent(nCode, wParam, lParam);
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

// Process keyboard events and forward to the layer controller
LRESULT KeyboardHook::HandleKeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam) {
    // Only process if nCode is HC_ACTION
    if (nCode != HC_ACTION) {
        return CallNextHookEx(hook_handle_, nCode, wParam, lParam);
    }

    const auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    
    // Ignore synthetic events we generated ourselves
    if (kbd->dwExtraInfo == kSyntheticEventTag) {
        return CallNextHookEx(hook_handle_, nCode, wParam, lParam);
    }

    const DWORD vkCode = kbd->vkCode;
    const DWORD scanCode = kbd->scanCode;
    
    // Determine if this is a key down or key up event
    bool pressed = false;
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
        pressed = true;
    } else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
        pressed = false;
    } else {
        // Unknown event type, pass through
        return CallNextHookEx(hook_handle_, nCode, wParam, lParam);
    }

    // Handle CapsLock specially
    if (vkCode == VK_CAPITAL) {
        if (HandleCapsLock(vkCode, pressed)) {
            return 1; // Swallow the event
        }
    }

    // Handle other keys
    if (HandleKey(vkCode, scanCode, pressed)) {
        return 1; // Swallow the event
    }

    // Pass through unhandled events
    return CallNextHookEx(hook_handle_, nCode, wParam, lParam);
}

// Track CapsLock press/release transitions
bool KeyboardHook::HandleCapsLock(DWORD vkCode, bool pressed) {
    if (vkCode != VK_CAPITAL) {
        return false;
    }
    
    UpdateCapsLockState(pressed);
    return true; // Always swallow CapsLock events
}

// Queries the current state of modifier keys using GetAsyncKeyState.
core::Modifiers KeyboardHook::GetCurrentModifiers() {
    core::Modifiers mods = core::Modifiers::None;
    
    // GetAsyncKeyState returns the key state at the time of the call.
    // The high bit indicates if the key is currently down.
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        mods = mods | core::Modifiers::Shift;
    }
    if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        mods = mods | core::Modifiers::Control;
    }
    if (GetAsyncKeyState(VK_MENU) & 0x8000) {  // VK_MENU is Alt
        mods = mods | core::Modifiers::Alt;
    }
    if ((GetAsyncKeyState(VK_LWIN) & 0x8000) || (GetAsyncKeyState(VK_RWIN) & 0x8000)) {
        mods = mods | core::Modifiers::Meta;
    }
    
    return mods;
}

// Forward key events to the layer controller
bool KeyboardHook::HandleKey(DWORD vkCode, DWORD scanCode, bool pressed) {
    if (!controller_) {
        return false;
    }

    const std::string token = ExtractKeyToken(vkCode, scanCode);
    if (token.empty()) {
        // Failed to derive a token; let the system handle it
        return false;
    }

    // Capture current modifier state so synthetic events can preserve it
    const core::Modifiers modifiers = GetCurrentModifiers();
    
    // Forward into the shared controller with app context and modifier state
    core::KeyEvent key_event{token, ResolveAppForEvent(), pressed, modifiers};
    return controller_->OnKeyEvent(key_event);
}

// Extract a normalized key token from virtual key code
std::string KeyboardHook::ExtractKeyToken(DWORD vkCode, DWORD scanCode) {
    // For letter keys (A-Z), return uppercase letter
    if (vkCode >= 'A' && vkCode <= 'Z') {
        return std::string(1, static_cast<char>(vkCode));
    }

    // For digit keys (0-9), return the digit
    if (vkCode >= '0' && vkCode <= '9') {
        return std::string(1, static_cast<char>(vkCode));
    }

    // For non-printable keys, return hex code
    std::ostringstream token;
    token << "0X" << std::uppercase << std::hex << vkCode;
    return token.str();
}

// Derives a normalized application identifier using the shared AppMonitor
std::string KeyboardHook::ResolveAppForEvent() {
    if (!app_monitor_) {
        return "";
    }
    return app_monitor_->CurrentAppName();
}

// Update CapsLock state and notify the controller
void KeyboardHook::UpdateCapsLockState(bool pressed) {
    if (pressed == capslock_down_) {
        return;
    }

    // Transition edge detected
    capslock_down_ = pressed;
    std::ostringstream msg;
    msg << "[Windows::KeyboardHook] CapsLock " << (pressed ? "pressed" : "released");
    if (app_monitor_) {
        const std::string focus = app_monitor_->CurrentAppName();
        if (!focus.empty()) {
            msg << " (focus=" << focus << ")";
        }
    }
    core::logging::Debug(msg.str());

    if (!controller_) {
        return;
    }

    if (pressed) {
        controller_->OnCapsLockPressed();
    } else {
        controller_->OnCapsLockReleased();
    }
}

} // namespace caps::platform::windows
