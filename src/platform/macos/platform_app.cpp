#include "platform_app.h"

// CapsUnlocked macOS entry adapter: manages hook installation, overlay view, and
// run-loop scaffolding for the macOS-specific executable.

#include <memory>
#include <stdexcept>

#include "core/app_context.h"
#include "core/layer/layer_controller.h"
#include "core/logging.h"
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

// Installs hooks and wires callbacks. Throws if the user has not granted permissions.
void PlatformApp::Initialize() {
    core::logging::Info("[macOS::PlatformApp] Initializing platform app");
    if (!keyboard_hook_->Install(context_.Layer())) {
        throw std::runtime_error(
            "CapsUnlocked needs Accessibility/Input Monitoring permission. Enable it in "
            "System Settings → Privacy & Security → Input Monitoring and restart the app.");
    }
    // When the layer resolves a mapping, immediately emit the CGEvent via Output.
    context_.Layer().SetActionCallback(
        [this](const std::string& action, bool pressed) { output_->Emit(action, pressed); });
}

// Starts listening for events and blocks inside CFRunLoopRun() until Shutdown() is called.
void PlatformApp::Run() {
    core::logging::Info("[macOS::PlatformApp] Entering run loop");
    run_loop_ = CFRunLoopGetCurrent();
    // After the hook is armed we block in CFRunLoopRun() until Shutdown() stops it.
    keyboard_hook_->StartListening();
    CFRunLoopRun();
}

// Stops the run loop, tears down hooks, and hides any overlay that might be showing.
void PlatformApp::Shutdown() {
    core::logging::Info("[macOS::PlatformApp] Shutting down platform app");
    if (run_loop_) {
        CFRunLoopStop(run_loop_);
        run_loop_ = nullptr;
    }
    // StopListening tears down the event tap and IOHID manager before exiting.
    keyboard_hook_->StopListening();
    overlay_view_->Hide();
}

} // namespace caps::platform::macos
