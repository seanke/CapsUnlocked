#include "platform_app.h"

// CapsUnlocked macOS entry adapter: manages hook installation, overlay view, and
// run-loop scaffolding for the macOS-specific executable.

#include <iostream>
#include <memory>

#include "core/app_context.h"
#include "core/layer/layer_controller.h"
#include "core/overlay/overlay_model.h"
#include "platform/macos/keyboard_hook.h"
#include "platform/macos/output.h"
#include "platform/macos/overlay_view.h"

namespace caps::platform::macos {

PlatformApp::PlatformApp(core::AppContext& context)
    : context_(context),
      keyboard_hook_(std::make_unique<KeyboardHook>()),
      output_(std::make_unique<Output>()),
      overlay_view_(std::make_unique<OverlayView>(context.Overlay())) {}

void PlatformApp::Initialize() {
    std::cout << "[macOS::PlatformApp] Initializing platform app" << std::endl;
    keyboard_hook_->Install(context_.Layer());
    // TODO: Validate permissions, prime overlay resources, and watch config changes.
}

void PlatformApp::Run() {
    std::cout << "[macOS::PlatformApp] Entering run loop stub" << std::endl;
    keyboard_hook_->StartListening();
    output_->Emit("simulate-arrow-down");
    // TODO: Enter CFRunLoopRun and process lifecycle events until shutdown.
}

void PlatformApp::Shutdown() {
    std::cout << "[macOS::PlatformApp] Shutting down platform app" << std::endl;
    keyboard_hook_->StopListening();
    overlay_view_->Hide();
    // TODO: Stop CFRunLoop, release event taps, and dispose of overlay resources.
}

} // namespace caps::platform::macos
