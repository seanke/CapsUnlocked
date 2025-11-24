#pragma once

#include <windows.h>
#include <string>

namespace caps::core {
class LayerController;
struct KeyEvent;
}

namespace caps::platform::windows {

// Win32 low-level keyboard hook adapter that forwards key events to LayerController.
class KeyboardHook {
public:
    void Install(core::LayerController& controller);
    void StartListening();
    void StopListening();

private:
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    LRESULT HandleKeyboardEvent(int nCode, WPARAM wParam, LPARAM lParam);
    bool HandleCapsLock(DWORD vkCode, bool pressed);
    bool HandleKey(DWORD vkCode, DWORD scanCode, bool pressed);
    static std::string ExtractKeyToken(DWORD vkCode, DWORD scanCode);
    void UpdateCapsLockState(bool pressed);

    core::LayerController* controller_{nullptr};
    HHOOK hook_handle_{nullptr};
    bool capslock_down_{false};
    
    static KeyboardHook* instance_;
};

// Tag value stored in dwExtraInfo to identify synthetic events we generate.
inline constexpr ULONG_PTR kSyntheticEventTag = 0x43415053; // 'CAPS'

} // namespace caps::platform::windows
