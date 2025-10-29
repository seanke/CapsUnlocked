#include "hook.h"
#include "input.h"
#include <cctype>

static bool g_capsLayerActive = false;
static KeyMapping g_layerMap;

void set_mapping(const KeyMapping& map) {
    g_layerMap = map;
    g_capsLayerActive = false;
}

#if defined(_WIN32)

static HHOOK g_hook = nullptr;

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    const KBDLLHOOKSTRUCT* pkb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    const bool isKeyUp = (pkb->flags & LLKHF_UP) != 0;
    const KeyCode vk = pkb->vkCode;

    if (is_our_injected(*pkb)) {
        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    if (vk == CAPS_LOCK_KEY) {
        g_capsLayerActive = !isKeyUp;
        return 1; // block default CapsLock behavior
    }

    if (g_capsLayerActive) {
        auto it = g_layerMap.find(vk);
        if (it == g_layerMap.end()) {
            if (vk >= 'a' && vk <= 'z') {
                const KeyCode upper = static_cast<KeyCode>(std::toupper(static_cast<int>(vk)));
                it = g_layerMap.find(upper);
            }
        }
        if (it != g_layerMap.end()) {
            send_key(it->second, !isKeyUp);
            return 1;
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

bool install_hook() {
    if (g_hook) return true;
    g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandleW(nullptr), 0);
    return g_hook != nullptr;
}

void uninstall_hook() {
    if (g_hook) {
        UnhookWindowsHookEx(g_hook);
        g_hook = nullptr;
    }
}

#else

static CFMachPortRef g_eventTap = nullptr;
static CFRunLoopSourceRef g_runLoopSource = nullptr;

static CGEventRef EventTapCallback(CGEventTapProxy, CGEventType type, CGEventRef event, void*) {
    if (type == kCGEventTapDisabledByTimeout || type == kCGEventTapDisabledByUserInput) {
        if (g_eventTap) {
            CGEventTapEnable(g_eventTap, true);
        }
        return event;
    }

    if (type == kCGEventFlagsChanged) {
        const KeyCode vk = static_cast<KeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
        if (vk == CAPS_LOCK_KEY) {
            const bool down = (CGEventGetFlags(event) & kCGEventFlagMaskAlphaShift) != 0;
            g_capsLayerActive = down;
            return nullptr; // swallow to keep caps off
        }
        return event;
    }

    if (type != kCGEventKeyDown && type != kCGEventKeyUp) {
        return event;
    }

    if (is_our_injected(event)) {
        return event;
    }

    const KeyCode vk = static_cast<KeyCode>(CGEventGetIntegerValueField(event, kCGKeyboardEventKeycode));
    const bool isKeyDown = (type == kCGEventKeyDown);

    if (vk == CAPS_LOCK_KEY) {
        g_capsLayerActive = isKeyDown;
        return nullptr;
    }

    if (g_capsLayerActive) {
        const auto it = g_layerMap.find(vk);
        if (it != g_layerMap.end()) {
            send_key(it->second, isKeyDown);
            return nullptr;
        }
    }
    return event;
}

bool install_hook() {
    if (g_eventTap) return true;

    const CGEventMask mask =
        CGEventMaskBit(kCGEventKeyDown) |
        CGEventMaskBit(kCGEventKeyUp) |
        CGEventMaskBit(kCGEventFlagsChanged);

    g_eventTap = CGEventTapCreate(kCGSessionEventTap,
                                  kCGHeadInsertEventTap,
                                  kCGEventTapOptionDefault,
                                  mask,
                                  EventTapCallback,
                                  nullptr);
    if (!g_eventTap) {
        return false;
    }

    g_runLoopSource = CFMachPortCreateRunLoopSource(kCFAllocatorDefault, g_eventTap, 0);
    if (!g_runLoopSource) {
        CFRelease(g_eventTap);
        g_eventTap = nullptr;
        return false;
    }

    CFRunLoopAddSource(CFRunLoopGetCurrent(), g_runLoopSource, kCFRunLoopCommonModes);
    CGEventTapEnable(g_eventTap, true);
    return true;
}

void uninstall_hook() {
    if (g_runLoopSource) {
        CFRunLoopRemoveSource(CFRunLoopGetCurrent(), g_runLoopSource, kCFRunLoopCommonModes);
        CFRelease(g_runLoopSource);
        g_runLoopSource = nullptr;
    }
    if (g_eventTap) {
        CGEventTapEnable(g_eventTap, false);
        CFRelease(g_eventTap);
        g_eventTap = nullptr;
    }
}

#endif
