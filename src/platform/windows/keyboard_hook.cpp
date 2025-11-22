#include "keyboard_hook.h"

// CapsUnlocked Windows adapter: stub low-level keyboard hook bridge that will
// feed key events into the shared layer controller.

#include "core/layer/layer_controller.h"
#include "core/logging.h"

namespace caps::platform::windows {

void KeyboardHook::Install(core::LayerController& controller) {
    controller_ = &controller;
    core::logging::Info("[Windows::KeyboardHook] Installing hook");
    // TODO: Register WH_KEYBOARD_LL hook and ensure thread message pump is ready.
}

void KeyboardHook::StartListening() {
    core::logging::Info("[Windows::KeyboardHook] Starting to listen for keyboard events");
    if (controller_) {
        // Simulate a quick Caps layer cycle so the scaffold exercises mapping callbacks.
        controller_->OnCapsLockPressed();
        controller_->OnKeyEvent({"H", "", true});
        controller_->OnCapsLockReleased();
    }
    // TODO: Enter message loop and translate Win32 events into controller calls.
}

void KeyboardHook::StopListening() {
    core::logging::Info("[Windows::KeyboardHook] Stopping keyboard listening");
    // TODO: Unhook Windows keyboard hook and signal message loop termination.
}

} // namespace caps::platform::windows
