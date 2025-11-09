#include "platform_app.h"

// CapsUnlocked macOS entry adapter: manages hook installation, overlay view, and
// run-loop scaffolding for the macOS-specific executable.

#include <iostream>
#include <memory>
#include <stdexcept>

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
    if (!keyboard_hook_->Install(context_.Layer())) {
        throw std::runtime_error(
            "CapsUnlocked needs Accessibility/Input Monitoring permission. Enable it in "
            "System Settings → Privacy & Security → Input Monitoring and restart the app.");
    }
    context_.Layer().SetActionCallback(
        [this](const std::string& action, bool pressed) { output_->Emit(action, pressed); });
}

void PlatformApp::Run() {
    std::cout << "[macOS::PlatformApp] Entering run loop" << std::endl;
    run_loop_ = CFRunLoopGetCurrent();
    keyboard_hook_->StartListening();
    CFRunLoopRun();
}

void PlatformApp::Shutdown() {
    std::cout << "[macOS::PlatformApp] Shutting down platform app" << std::endl;
    if (run_loop_) {
        CFRunLoopStop(run_loop_);
        run_loop_ = nullptr;
    }
    keyboard_hook_->StopListening();
    overlay_view_->Hide();
}

} // namespace caps::platform::macos
