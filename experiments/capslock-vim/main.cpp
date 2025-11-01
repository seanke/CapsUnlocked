#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDManager.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/hidsystem/IOHIDLib.h>

#include <array>
#include <atomic>
#include <iomanip>
#include <iostream>
#include <optional>

namespace {

constexpr CGKeyCode kCapsLockKey = kVK_CapsLock;

struct RemapEntry {
    CGKeyCode source;
    CGKeyCode target;
};

constexpr std::array<RemapEntry, 4> kVimRemaps{{
    {kVK_ANSI_H, kVK_LeftArrow},
    {kVK_ANSI_J, kVK_DownArrow},
    {kVK_ANSI_K, kVK_UpArrow},
    {kVK_ANSI_L, kVK_RightArrow},
}};

std::atomic<bool> g_capsHeld{false};
CFMachPortRef g_eventTap = nullptr;
IOHIDManagerRef g_hidManager = nullptr;

std::optional<CGKeyCode> remap_key(CGKeyCode keycode) {
    for (const auto& entry : kVimRemaps) {
        if (entry.source == keycode) {
            return entry.target;
        }
    }
    return std::nullopt;
}

void hid_input_callback(void*, IOReturn result, void*, IOHIDValueRef value) {
    if (result != kIOReturnSuccess || !value) {
        return;
    }
    IOHIDElementRef element = IOHIDValueGetElement(value);
    if (!element) {
        return;
    }
    if (IOHIDElementGetUsagePage(element) != kHIDPage_KeyboardOrKeypad) {
        return;
    }
    if (IOHIDElementGetUsage(element) != kHIDUsage_KeyboardCapsLock) {
        return;
    }

    const CFIndex pressed = IOHIDValueGetIntegerValue(value);
    g_capsHeld.store(pressed != 0, std::memory_order_relaxed);
}

CFMutableDictionaryRef create_device_matching_dict(uint32_t usage_page, uint32_t usage) {
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                            0,
                                                            &kCFTypeDictionaryKeyCallBacks,
                                                            &kCFTypeDictionaryValueCallBacks);
    if (!dict) {
        return nullptr;
    }

    CFNumberRef page_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage_page);
    CFNumberRef usage_ref = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &usage);
    if (!page_ref || !usage_ref) {
        if (page_ref) CFRelease(page_ref);
        if (usage_ref) CFRelease(usage_ref);
        CFRelease(dict);
        return nullptr;
    }

    CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), page_ref);
    CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), usage_ref);

    CFRelease(page_ref);
    CFRelease(usage_ref);

    return dict;
}

bool has_accessibility_permission() {
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 1090)
    if (&AXIsProcessTrustedWithOptions != nullptr) {
        const void* keys[] = { kAXTrustedCheckOptionPrompt };
        const void* values[] = { kCFBooleanFalse };
        CFDictionaryRef options = CFDictionaryCreate(kCFAllocatorDefault,
                                                     keys,
                                                     values,
                                                     1,
                                                     &kCFTypeDictionaryKeyCallBacks,
                                                     &kCFTypeDictionaryValueCallBacks);
        const bool trusted = AXIsProcessTrustedWithOptions(options);
        if (options) CFRelease(options);
        return trusted;
    }
#endif
    return AXIsProcessTrusted();
}

bool has_input_monitoring_permission() {
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101500)
    if (&IOHIDCheckAccess != nullptr) {
        return IOHIDCheckAccess(kIOHIDRequestTypeListenEvent) == kIOHIDAccessTypeGranted;
    }
#endif
    return true;
}

CGEventRef event_callback(CGEventTapProxy,
                          CGEventType type,
                          CGEventRef event,
                          void*) {
    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        if (g_eventTap) {
            CGEventTapEnable(g_eventTap, true);
        }
        return event;
    }

    if (!event) {
        return event;
    }

    if (type == kCGEventFlagsChanged) {
        const CGKeyCode keycode = static_cast<CGKeyCode>(
            CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
        if (keycode == kCapsLockKey) {
            const CGEventFlags flags = CGEventGetFlags(event);
            const bool caps_active = (flags & kCGEventFlagMaskAlphaShift) != 0;
            g_capsHeld.store(caps_active, std::memory_order_relaxed);
            return nullptr; // swallow so CapsLock state never toggles
        }
        return event;
    }

    if (type != kCGEventKeyDown && type != kCGEventKeyUp) {
        return event;
    }

    const CGKeyCode keycode = static_cast<CGKeyCode>(
        CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));

    if (keycode == kCapsLockKey) {
        g_capsHeld.store(type == kCGEventKeyDown, std::memory_order_relaxed);
        return nullptr;
    }

    if (!g_capsHeld.load(std::memory_order_relaxed)) {
        return event;
    }

    const std::optional<CGKeyCode> target = remap_key(keycode);
    if (!target.has_value()) {
        return event;
    }

    CGEventSetIntegerValueField(event, kCGKeyboardEventKeycode, target.value());
    CGEventSetFlags(event, CGEventGetFlags(event) & ~kCGEventFlagMaskAlphaShift);

    return event;
}

} // namespace

int main() {
    if (!has_accessibility_permission()) {
        std::cerr << "Accessibility permission missing. Enable it in System Settings > Privacy & Security > Accessibility." << std::endl;
        return 1;
    }
    if (!has_input_monitoring_permission()) {
        std::cerr << "Input Monitoring permission missing. Enable it in System Settings > Privacy & Security > Input Monitoring." << std::endl;
        return 1;
    }

    const CGEventMask mask =
        CGEventMaskBit(kCGEventKeyDown) |
        CGEventMaskBit(kCGEventKeyUp) |
        CGEventMaskBit(kCGEventFlagsChanged);

    g_eventTap = CGEventTapCreate(kCGHIDEventTap,
                                  kCGHeadInsertEventTap,
                                  kCGEventTapOptionDefault,
                                  mask,
                                  event_callback,
                                  nullptr);

    if (!g_eventTap) {
        std::cerr << "HID event tap failed (need Input Monitoring or root). "
                  << "Falling back to session event tap; remapping may be limited." << std::endl;
        g_eventTap = CGEventTapCreate(kCGSessionEventTap,
                                      kCGHeadInsertEventTap,
                                      kCGEventTapOptionDefault,
                                      mask,
                                      event_callback,
                                      nullptr);
    }

    if (!g_eventTap) {
        std::cerr << "Failed to create event tap. Check permissions and try again." << std::endl;
        return 1;
    }

    g_hidManager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (g_hidManager) {
        CFMutableDictionaryRef keyboard_match = create_device_matching_dict(kHIDPage_GenericDesktop,
                                                                            kHIDUsage_GD_Keyboard);
        if (keyboard_match) {
            const void* matches[] = { keyboard_match };
            CFArrayRef match_array = CFArrayCreate(kCFAllocatorDefault,
                                                   matches,
                                                   1,
                                                   &kCFTypeArrayCallBacks);
            if (match_array) {
                IOHIDManagerSetDeviceMatchingMultiple(g_hidManager, match_array);
                CFRelease(match_array);
            }
            CFRelease(keyboard_match);
        }

        IOHIDManagerRegisterInputValueCallback(g_hidManager, hid_input_callback, nullptr);
        IOHIDManagerScheduleWithRunLoop(g_hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        const IOReturn open_result = IOHIDManagerOpen(g_hidManager, kIOHIDOptionsTypeNone);
        if (open_result != kIOReturnSuccess) {
            std::cerr << "Warning: IOHIDManager open failed (" << std::hex << open_result
                      << "). CapsLock detection may not work when remapped." << std::dec << std::endl;
        }
    } else {
        std::cerr << "Warning: Failed to create IOHIDManager. CapsLock detection may not work when remapped."
                  << std::endl;
    }

    CFRunLoopSourceRef runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault,
                                                                     g_eventTap,
                                                                     0);
    if (!runLoopSource) {
        std::cerr << "Failed to create run loop source." << std::endl;
        CFRelease(g_eventTap);
        g_eventTap = nullptr;
        return 1;
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(g_eventTap, true);

    std::cout << "capslock-vim running. Hold CapsLock and use HJKL as arrow keys. Ctrl+C to exit."
              << std::endl;

    CFRunLoopRun();

    CGEventTapEnable(g_eventTap, false);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CFRelease(runLoopSource);
    CFRelease(g_eventTap);
    g_eventTap = nullptr;

    if (g_hidManager) {
        IOHIDManagerUnscheduleFromRunLoop(g_hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        IOHIDManagerClose(g_hidManager, kIOHIDOptionsTypeNone);
        CFRelease(g_hidManager);
        g_hidManager = nullptr;
    }

    return 0;
}
