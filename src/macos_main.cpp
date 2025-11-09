#include <iostream>

// CapsUnlocked macOS executable entry point: wires the core app context to the
// macOS platform adapter and drives the skeleton lifecycle.
#include <string>

#include "core/app_context.h"
#include "platform/macos/platform_app.h"

int main(int argc, char* argv[]) {
    std::cout << "[macOS::Main] Bootstrapping CapsUnlocked skeleton" << std::endl;

    // Default to the adjacent config, mirroring how the Windows build behaves.
    std::string config_path = "capsunlocked.ini";
    if (argc > 1) {
        // Allow overriding the config path via CLI for demos/tests.
        config_path = argv[1];
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

    std::cout << "[macOS::Main] Exiting skeleton" << std::endl;
    return 0;
}
