#include "keyboard_hook.h"

// CapsUnlocked macOS adapter: placeholder CGEvent tap bridge that will stream
// keyboard events and CapsLock state into the shared layer controller.

#include <iostream>

#include "core/layer/layer_controller.h"

namespace caps::platform::macos {

void KeyboardHook::Install(core::LayerController& controller) {
    controller_ = &controller;
    std::cout << "[macOS::KeyboardHook] Installing event tap stub" << std::endl;
    // TODO: Create CGEventTap and IOHID listener, requesting accessibility permissions.
}

void KeyboardHook::StartListening() {
    std::cout << "[macOS::KeyboardHook] Starting run loop stub" << std::endl;
    if (controller_) {
        controller_->OnCapsLockPressed();
        controller_->OnKeyEvent({"J", true});
        controller_->OnCapsLockReleased();
    }
    // TODO: Schedule tap on CFRunLoop and forward events into controller callbacks.
}

void KeyboardHook::StopListening() {
    std::cout << "[macOS::KeyboardHook] Stopping event tap stub" << std::endl;
    // TODO: Disable CGEventTap, unschedule HID manager, and clean up run-loop sources.
}

} // namespace caps::platform::macos
