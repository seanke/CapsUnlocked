#pragma once

#include <windows.h>
#include <memory>
#include <string>

#include "platform/windows/app_monitor.h"
#include "platform/windows/keyboard_hook.h"
#include "platform/windows/output.h"

namespace caps::core {
class AppContext;
}

namespace caps::platform::windows {

// Windows platform adapter: manages hook installation and message loop.
class PlatformApp {
public:
    explicit PlatformApp(core::AppContext& context);

    void Initialize();
    void Run();
    void Shutdown();

private:
    core::AppContext& context_;
    std::unique_ptr<AppMonitor> app_monitor_;
    std::unique_ptr<KeyboardHook> keyboard_hook_;
    std::unique_ptr<Output> output_;
    DWORD main_thread_id_{0};
};

} // namespace caps::platform::windows
