#include "platform_app.h"

// CapsUnlocked Windows entry adapter: coordinates hook setup and
// run-loop scaffolding for the Windows-specific executable.

#include <windows.h>
#include <memory>

#include "core/app_context.h"
#include "core/layer/layer_controller.h"
#include "core/logging.h"
#include "platform/windows/app_monitor.h"
#include "platform/windows/keyboard_hook.h"
#include "platform/windows/output.h"

namespace caps::platform::windows {

PlatformApp::PlatformApp(core::AppContext& context)
    : context_(context),
      app_monitor_(std::make_unique<AppMonitor>()),
      keyboard_hook_(std::make_unique<KeyboardHook>(app_monitor_.get())),
      output_(std::make_unique<Output>()) {}

// Establishes hooks and wiring so mapped actions get emitted via Output.
void PlatformApp::Initialize() {
    core::logging::Info("[Windows::PlatformApp] Initializing platform app");
    main_thread_id_ = GetCurrentThreadId();
    keyboard_hook_->Install(context_.Layer());
    context_.Layer().SetActionCallback(
        [this](const std::string& action, bool pressed) { output_->Emit(action, pressed); });
}

// Runs the Windows message loop to process keyboard hook events.
void PlatformApp::Run() {
    core::logging::Info("[Windows::PlatformApp] Running message loop");
    keyboard_hook_->StartListening();
    
    // Run the Windows message loop
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    core::logging::Info("[Windows::PlatformApp] Message loop exited");
}

// Cleans up hook state so Windows releases our low-level listeners cleanly.
void PlatformApp::Shutdown() {
    core::logging::Info("[Windows::PlatformApp] Shutting down platform app");
    keyboard_hook_->StopListening();
    
    // Post WM_QUIT to exit the message loop if it's running
    if (main_thread_id_ != 0) {
        PostThreadMessage(main_thread_id_, WM_QUIT, 0, 0);
    }
}

} // namespace caps::platform::windows
