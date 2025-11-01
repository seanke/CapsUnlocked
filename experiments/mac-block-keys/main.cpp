#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <IOKit/hidsystem/IOHIDLib.h>
#include <atomic>
#include <iostream>

namespace {

constexpr CGKeyCode kCapsLockKey = kVK_CapsLock;
std::atomic<bool> g_capsHeld{false};
CFMachPortRef g_eventTap = nullptr;

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

CGEventRef event_callback(CGEventTapProxy, CGEventType type, CGEventRef event, void*) {
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
            g_capsHeld.store((flags & kCGEventFlagMaskAlphaShift) != 0, std::memory_order_relaxed);
            return nullptr;
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

    if (g_capsHeld.load(std::memory_order_relaxed)) {
        return nullptr;
    }

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
        std::cerr << "HID-level event tap failed (need Input Monitoring or root). "
                  << "Falling back to session tap; blocking may be limited." << std::endl;
        g_eventTap = CGEventTapCreate(kCGSessionEventTap,
                                      kCGHeadInsertEventTap,
                                      kCGEventTapOptionDefault,
                                      mask,
                                      event_callback,
                                      nullptr);
    }

    if (!g_eventTap) {
        std::cerr << "Failed to create event tap. Check accessibility permissions." << std::endl;
        return 1;
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

    std::cout << "mac-block-keys running. Hold CapsLock to block other keys. Ctrl+C to exit."
              << std::endl;

    CFRunLoopRun();

    CGEventTapEnable(g_eventTap, false);
    CFRunLoopRemoveSource(CFRunLoopGetCurrent(), runLoopSource, kCFRunLoopCommonModes);
    CFRelease(runLoopSource);
    CFRelease(g_eventTap);
    g_eventTap = nullptr;

    return 0;
}
