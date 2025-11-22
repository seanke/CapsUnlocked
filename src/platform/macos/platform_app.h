#pragma once

#include <CoreFoundation/CoreFoundation.h>

#include <memory>
#include <string>

#include "platform/macos/app_monitor.h"
#include "platform/macos/keyboard_hook.h"
#include "platform/macos/output.h"

namespace caps::core {
class AppContext;
}

namespace caps::platform::macos {

// macOS entry adapter: wires hooks/output together and runs the CFRunLoop.
class PlatformApp {
public:
    explicit PlatformApp(core::AppContext& context);

    void Initialize();
    void Run();
    void Shutdown();

private:
    core::AppContext& context_;
    std::unique_ptr<AppMonitor> app_monitor_;     // Reports frontmost app.
    std::unique_ptr<KeyboardHook> keyboard_hook_; // Captures hardware events.
    std::unique_ptr<Output> output_;              // Emits mapped CGEvents.
    CFRunLoopRef run_loop_{nullptr};
};

} // namespace caps::platform::macos
