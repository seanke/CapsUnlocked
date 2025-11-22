#pragma once

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDManager.h>

#include <string>

namespace caps::core {
class LayerController;
struct KeyEvent;
} // namespace caps::core

namespace caps::platform::macos {

// Owns the CGEvent tap plus IOHID listener that forward CapsLock + keyboard events
// into the shared LayerController.
class KeyboardHook {
public:
    KeyboardHook() = default;
    ~KeyboardHook();

    // Configures the event tap and IOHID monitor. Returns false if the caller needs to
    // prompt the user for accessibility/input monitoring permissions.
    bool Install(core::LayerController& controller);
    // Registers the tap with the current CFRunLoop and begins listening for events.
    void StartListening();
    // Removes the tap, unschedules IOHID callbacks, and releases all CF resources.
    // Safe to call multiple times (e.g., during teardown or error handling).
    void StopListening();

private:
    static CGEventRef EventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon);
    CGEventRef HandleEvent(CGEventRef event, CGEventType type);
    bool HandleCapsLock(CGEventRef event);
    bool HandleKey(CGEventRef event, bool pressed);
    static std::string ExtractKeyToken(CGEventRef event);
    static std::string ResolveAppForEvent(CGEventRef event);
    bool EnsureAccessibilityPrivileges() const;
    bool EnsureInputMonitoringPrivileges() const;
    // Normalizes CapsLock transitions to a single place so both CGEvent and IOHID paths reuse it.
    void UpdateCapsLockState(bool pressed);
    // IOHID plumbing that mirrors the standalone capslock-vim experiment.
    bool InitializeHIDMonitor();
    void ShutdownHIDMonitor();
    static void HidInputCallback(void* context, IOReturn result, void* sender, IOHIDValueRef value);
    void HandleHidValue(IOReturn result, IOHIDValueRef value);
    // Small helper for building device matching dictionaries (usage page + usage).
    static CFMutableDictionaryRef CreateDeviceMatchingDict(uint32_t usage_page, uint32_t usage);

    core::LayerController* controller_{nullptr}; // Not owned; lives in AppContext.
    CFMachPortRef event_tap_{nullptr};
    CFRunLoopSourceRef run_loop_source_{nullptr};
    IOHIDManagerRef hid_manager_{nullptr};
    CFRunLoopRef scheduled_run_loop_{nullptr};
    bool hid_scheduled_{false};
    bool hid_open_{false};
    bool capslock_down_{false};
};

} // namespace caps::platform::macos
