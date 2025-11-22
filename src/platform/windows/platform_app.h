#pragma once

#include <memory>
#include <string>

#include "platform/windows/keyboard_hook.h"
#include "platform/windows/output.h"

namespace caps::core {
class AppContext;
}

namespace caps::platform::windows {

// Windows twin to the mac adapter; currently scaffolding but documents lifecycle.
class PlatformApp {
public:
    explicit PlatformApp(core::AppContext& context);

    void Initialize();
    void Run();
    void Shutdown();

private:
    core::AppContext& context_;
    std::unique_ptr<KeyboardHook> keyboard_hook_;
    std::unique_ptr<Output> output_;
};

} // namespace caps::platform::windows
