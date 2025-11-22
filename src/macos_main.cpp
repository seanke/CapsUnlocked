// CapsUnlocked macOS executable entry point: wires the core app context to the
// macOS platform adapter and drives the skeleton lifecycle.
#include <string>
#include <string_view>

#include "core/app_context.h"
#include "core/logging.h"
#include "platform/macos/platform_app.h"

int main(int argc, char* argv[]) {
    caps::core::logging::Info("[macOS::Main] Bootstrapping CapsUnlocked skeleton");

    // Default to the adjacent config, mirroring how the Windows build behaves.
    std::string config_path = "capsunlocked.ini";
    for (int i = 1; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg.rfind("--log-level=", 0) == 0) {
            const auto level_name = arg.substr(std::string_view("--log-level=").size());
            if (const auto parsed = caps::core::logging::ParseLevel(level_name)) {
                caps::core::logging::SetLevel(*parsed);
                caps::core::logging::Info("[macOS::Main] Log level set to " + std::string(level_name));
            } else {
                caps::core::logging::Warn("[macOS::Main] Unknown log level '" + std::string(level_name) +
                                          "' (valid: debug, info, warn, error)");
            }
            continue;
        }

        // First non-flag argument is treated as config path override.
        config_path = arg;
    }
    // TODO: Surface CLI options (e.g., config override, diagnostics) and handle errors.

    // The AppContext owns all core subsystems (config, mapping, overlay, controller).
    caps::core::AppContext context;
    context.Initialize(config_path);

    // PlatformApp wires macOS-specific hooks/output/overlay onto the shared core.
    caps::platform::macos::PlatformApp platform_app(context);
    platform_app.Initialize();
    platform_app.Run();       // Blocks inside CFRunLoopRun() until Shutdown() is called.
    platform_app.Shutdown();  // Ensures hooks/overlay are torn down before exit.

    caps::core::logging::Info("[macOS::Main] Exiting skeleton");
    return 0;
}
