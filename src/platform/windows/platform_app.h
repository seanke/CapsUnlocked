#pragma once

#include <memory>
#include <string>

#include "platform/windows/keyboard_hook.h"
#include "platform/windows/output.h"
#include "platform/windows/overlay_view.h"

namespace caps::core {
class AppContext;
}

namespace caps::platform::windows {

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
    std::unique_ptr<OverlayView> overlay_view_;
};

} // namespace caps::platform::windows
