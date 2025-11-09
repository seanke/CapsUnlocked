#include "keyboard_hook.h"

#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDKeys.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDUsageTables.h>

#include <cctype>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>

#include "core/layer/layer_controller.h"
#include "platform/macos/event_tag.h"

namespace caps::platform::macos {

KeyboardHook::~KeyboardHook() {
    StopListening();
}

bool KeyboardHook::Install(core::LayerController& controller) {
    controller_ = &controller;

    if (!EnsureAccessibilityPrivileges()) {
        std::cerr << "[macOS::KeyboardHook] Accessibility permission is required. "
                     "Enable CapsUnlocked under System Settings → Privacy & Security → Accessibility."
                  << std::endl;
        return false;
    }

    if (!EnsureInputMonitoringPrivileges()) {
        std::cerr << "[macOS::KeyboardHook] Input Monitoring permission is required. "
                     "Enable CapsUnlocked under System Settings → Privacy & Security → Input Monitoring."
                  << std::endl;
        return false;
    }

    const CGEventMask mask = CGEventMaskBit(kCGEventKeyDown) |
                             CGEventMaskBit(kCGEventKeyUp) |
                             CGEventMaskBit(kCGEventFlagsChanged);

    event_tap_ = CGEventTapCreate(kCGHIDEventTap,
                                  kCGHeadInsertEventTap,
                                  kCGEventTapOptionDefault,
                                  mask,
                                  &KeyboardHook::EventCallback,
                                  this);
    if (!event_tap_) {
        std::cerr << "[macOS::KeyboardHook] Failed to create HID-level event tap; "
                     "falling back to session tap (may not block elevated apps)."
                  << std::endl;
        event_tap_ = CGEventTapCreate(kCGSessionEventTap,
                                      kCGHeadInsertEventTap,
                                      kCGEventTapOptionDefault,
                                      mask,
                                      &KeyboardHook::EventCallback,
                                      this);
    }

    if (!event_tap_) {
        std::cerr << "[macOS::KeyboardHook] Could not create any event tap. "
                     "Check Accessibility/Input Monitoring permissions."
                  << std::endl;
        return false;
    }

    run_loop_source_ = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, event_tap_, 0);
    if (!run_loop_source_) {
        std::cerr << "[macOS::KeyboardHook] Failed to create run-loop source for event tap" << std::endl;
        CFRelease(event_tap_);
        event_tap_ = nullptr;
        return false;
    }

    if (!InitializeHIDMonitor()) {
        std::cerr << "[macOS::KeyboardHook] Warning: IOHID manager unavailable; "
                     "CapsLock detection may fail if the key is remapped to No Action."
                  << std::endl;
    }

    return true;
}

void KeyboardHook::StartListening() {
    if (!event_tap_ || !run_loop_source_) {
        return;
    }

    scheduled_run_loop_ = CFRunLoopGetCurrent();
    CFRunLoopAddSource(scheduled_run_loop_, run_loop_source_, kCFRunLoopCommonModes);
    CGEventTapEnable(event_tap_, true);

    if (hid_manager_) {
        IOHIDManagerScheduleWithRunLoop(hid_manager_, scheduled_run_loop_, kCFRunLoopDefaultMode);
        hid_scheduled_ = true;

        const IOReturn open_result = IOHIDManagerOpen(hid_manager_, kIOHIDOptionsTypeNone);
        if (open_result != kIOReturnSuccess) {
            std::cerr << "[macOS::KeyboardHook] Warning: IOHIDManagerOpen failed (0x" << std::hex
                      << open_result << std::dec << "); CapsLock detection may be inconsistent."
                      << std::endl;
        } else {
            hid_open_ = true;
        }
    }
}

void KeyboardHook::StopListening() {
    ShutdownHIDMonitor();

    if (run_loop_source_ && scheduled_run_loop_) {
        CFRunLoopRemoveSource(scheduled_run_loop_, run_loop_source_, kCFRunLoopCommonModes);
        CFRelease(run_loop_source_);
        run_loop_source_ = nullptr;
    }

    if (event_tap_) {
        CGEventTapEnable(event_tap_, false);
        CFRelease(event_tap_);
        event_tap_ = nullptr;
    }

    scheduled_run_loop_ = nullptr;
}

CGEventRef KeyboardHook::EventCallback(CGEventTapProxy, CGEventType type, CGEventRef event, void* refcon) {
    auto* self = static_cast<KeyboardHook*>(refcon);
    if (!self) {
        return event;
    }

    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        if (self->event_tap_) {
            CGEventTapEnable(self->event_tap_, true);
        }
        return event;
    }

    return self->HandleEvent(event, type);
}

CGEventRef KeyboardHook::HandleEvent(CGEventRef event, CGEventType type) {
    if (!controller_) {
        return event;
    }

    if (event) {
        const int64_t tag = CGEventGetIntegerValueField(event, kCGEventSourceUserData);
        if (tag == kSyntheticEventTag) {
            return event; // allow synthetic events we emitted to pass through untouched
        }
    }

    switch (type) {
        case kCGEventFlagsChanged:
            if (HandleCapsLock(event)) {
                return nullptr;
            }
            break;
        case kCGEventKeyDown:
            if (HandleKey(event, true)) {
                return nullptr;
            }
            break;
        case kCGEventKeyUp:
            if (HandleKey(event, false)) {
                return nullptr;
            }
            break;
        default:
            break;
    }

    return event;
}

bool KeyboardHook::HandleCapsLock(CGEventRef event) {
    const CGKeyCode keycode =
        static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    if (keycode != kVK_CapsLock) {
        return false;
    }

    const bool pressed = (CGEventGetFlags(event) & kCGEventFlagMaskAlphaShift) != 0;
    UpdateCapsLockState(pressed);
    return true;
}

bool KeyboardHook::HandleKey(CGEventRef event, bool pressed) {
    const CGKeyCode keycode =
        static_cast<CGKeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    if (keycode == kVK_CapsLock) {
        UpdateCapsLockState(pressed);
        return true;
    }

    const std::string token = ExtractKeyToken(event);
    if (token.empty()) {
        return false;
    }

    core::KeyEvent key_event{token, pressed};
    return controller_->OnKeyEvent(key_event);
}

std::string KeyboardHook::ExtractKeyToken(CGEventRef event) {
    UniChar buffer[4];
    UniCharCount length = 0;
    CGEventKeyboardGetUnicodeString(event, std::size(buffer), &length, buffer);
    if (length > 0) {
        const UniChar code_point = buffer[0];
        if (code_point < 128) {
            char ascii = static_cast<char>(code_point);
            if (std::isalpha(static_cast<unsigned char>(ascii))) {
                ascii = static_cast<char>(std::toupper(static_cast<unsigned char>(ascii)));
            }
            return std::string(1, ascii);
        }
    }

    const auto keycode =
        static_cast<unsigned int>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));

    std::ostringstream token;
    token << "0X" << std::uppercase << std::hex << keycode;
    return token.str();
}

void KeyboardHook::UpdateCapsLockState(bool pressed) {
    if (pressed == capslock_down_) {
        return;
    }

    capslock_down_ = pressed;
    if (!controller_) {
        return;
    }

    if (pressed) {
        controller_->OnCapsLockPressed();
    } else {
        controller_->OnCapsLockReleased();
    }
}

bool KeyboardHook::InitializeHIDMonitor() {
    hid_manager_ = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (!hid_manager_) {
        return false;
    }

    CFMutableDictionaryRef keyboard_match =
        CreateDeviceMatchingDict(kHIDPage_GenericDesktop, kHIDUsage_GD_Keyboard);
    if (keyboard_match) {
        const void* matches[] = {keyboard_match};
        CFArrayRef match_array = CFArrayCreate(kCFAllocatorDefault, matches, 1, &kCFTypeArrayCallBacks);
        if (match_array) {
            IOHIDManagerSetDeviceMatchingMultiple(hid_manager_, match_array);
            CFRelease(match_array);
        }
        CFRelease(keyboard_match);
    }

    IOHIDManagerRegisterInputValueCallback(hid_manager_, &KeyboardHook::HidInputCallback, this);
    return true;
}

void KeyboardHook::ShutdownHIDMonitor() {
    if (!hid_manager_) {
        return;
    }

    if (hid_scheduled_ && scheduled_run_loop_) {
        IOHIDManagerUnscheduleFromRunLoop(hid_manager_, scheduled_run_loop_, kCFRunLoopDefaultMode);
    }
    if (hid_open_) {
        IOHIDManagerClose(hid_manager_, kIOHIDOptionsTypeNone);
    }

    CFRelease(hid_manager_);
    hid_manager_ = nullptr;
    hid_scheduled_ = false;
    hid_open_ = false;
}

void KeyboardHook::HidInputCallback(void* context, IOReturn result, void*, IOHIDValueRef value) {
    auto* self = static_cast<KeyboardHook*>(context);
    if (!self) {
        return;
    }
    self->HandleHidValue(result, value);
}

void KeyboardHook::HandleHidValue(IOReturn result, IOHIDValueRef value) {
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

    const bool pressed = IOHIDValueGetIntegerValue(value) != 0;
    UpdateCapsLockState(pressed);
}

CFMutableDictionaryRef KeyboardHook::CreateDeviceMatchingDict(uint32_t usage_page, uint32_t usage) {
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
        if (page_ref) {
            CFRelease(page_ref);
        }
        if (usage_ref) {
            CFRelease(usage_ref);
        }
        CFRelease(dict);
        return nullptr;
    }

    CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsagePageKey), page_ref);
    CFDictionarySetValue(dict, CFSTR(kIOHIDDeviceUsageKey), usage_ref);

    CFRelease(page_ref);
    CFRelease(usage_ref);
    return dict;
}

bool KeyboardHook::EnsureAccessibilityPrivileges() const {
    const void* keys[] = {kAXTrustedCheckOptionPrompt};
    const void* values[] = {kCFBooleanTrue};

    CFDictionaryRef options =
        CFDictionaryCreate(kCFAllocatorDefault,
                           keys,
                           values,
                           1,
                           &kCFCopyStringDictionaryKeyCallBacks,
                           &kCFTypeDictionaryValueCallBacks);
    if (!options) {
        return AXIsProcessTrusted();
    }

    const bool trusted = AXIsProcessTrustedWithOptions(options);
    CFRelease(options);
    return trusted;
}

bool KeyboardHook::EnsureInputMonitoringPrivileges() const {
#if defined(__MAC_OS_X_VERSION_MAX_ALLOWED) && (__MAC_OS_X_VERSION_MAX_ALLOWED >= 101500) && \
    defined(kIOHIDRequestTypeListenEvent) && defined(kIOHIDAccessTypeGranted)
    if (&IOHIDCheckAccess != nullptr) {
        return IOHIDCheckAccess(kIOHIDRequestTypeListenEvent) == kIOHIDAccessTypeGranted;
    }
#endif
    return true;
}

} // namespace caps::platform::macos
