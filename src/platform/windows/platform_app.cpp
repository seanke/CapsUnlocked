#include "platform_app.h"

// CapsUnlocked Windows entry adapter: coordinates hook setup, overlay view, and
// run-loop scaffolding for the Windows-specific executable.

#include <iostream>
#include <memory>

#include "core/app_context.h"
#include "core/layer/layer_controller.h"
#include "core/overlay/overlay_model.h"
#include "platform/windows/keyboard_hook.h"
#include "platform/windows/output.h"
#include "platform/windows/overlay_view.h"

namespace caps::platform::windows {

PlatformApp::PlatformApp(core::AppContext& context)
    : context_(context),
      keyboard_hook_(std::make_unique<KeyboardHook>()),
      output_(std::make_unique<Output>()),
      overlay_view_(std::make_unique<OverlayView>(context.Overlay())) {}

void PlatformApp::Initialize() {
    std::cout << "[Windows::PlatformApp] Initializing platform app" << std::endl;
    keyboard_hook_->Install(context_.Layer());
    context_.Layer().SetActionCallback(
        [this](const std::string& action, bool pressed) { output_->Emit(action, pressed); });
    // TODO: Load configuration, set up message loop, register overlay hotkeys.
}

void PlatformApp::Run() {
    std::cout << "[Windows::PlatformApp] Running message loop stub" << std::endl;
    keyboard_hook_->StartListening();
    // TODO: Pump Windows messages until shutdown is requested.
}

void PlatformApp::Shutdown() {
    std::cout << "[Windows::PlatformApp] Shutting down platform app" << std::endl;
    keyboard_hook_->StopListening();
    overlay_view_->Hide();
    // TODO: Release Win32 resources, unhook keyboard, and flush overlay state.
}

} // namespace caps::platform::windows
