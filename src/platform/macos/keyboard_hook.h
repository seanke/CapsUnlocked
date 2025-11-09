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

class KeyboardHook {
public:
    KeyboardHook() = default;
    ~KeyboardHook();

    bool Install(core::LayerController& controller);
    void StartListening();
    void StopListening();

private:
    static CGEventRef EventCallback(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void* refcon);
    CGEventRef HandleEvent(CGEventRef event, CGEventType type);
    bool HandleCapsLock(CGEventRef event);
    bool HandleKey(CGEventRef event, bool pressed);
    static std::string ExtractKeyToken(CGEventRef event);
    bool EnsureAccessibilityPrivileges() const;
    bool EnsureInputMonitoringPrivileges() const;
    void UpdateCapsLockState(bool pressed);
    bool InitializeHIDMonitor();
    void ShutdownHIDMonitor();
    static void HidInputCallback(void* context, IOReturn result, void* sender, IOHIDValueRef value);
    void HandleHidValue(IOReturn result, IOHIDValueRef value);
    static CFMutableDictionaryRef CreateDeviceMatchingDict(uint32_t usage_page, uint32_t usage);

    core::LayerController* controller_{nullptr};
    CFMachPortRef event_tap_{nullptr};
    CFRunLoopSourceRef run_loop_source_{nullptr};
    IOHIDManagerRef hid_manager_{nullptr};
    CFRunLoopRef scheduled_run_loop_{nullptr};
    bool hid_scheduled_{false};
    bool hid_open_{false};
    bool capslock_down_{false};
};

} // namespace caps::platform::macos
