#include <iostream>

// CapsUnlocked macOS executable entry point: wires the core app context to the
// macOS platform adapter and drives the skeleton lifecycle.
#include <string>

#include "core/app_context.h"
#include "platform/macos/platform_app.h"

int main(int argc, char* argv[]) {
    std::cout << "[macOS::Main] Bootstrapping CapsUnlocked skeleton" << std::endl;

    std::string config_path = "capsunlocked.ini";
    if (argc > 1) {
        config_path = argv[1];
    }
    // TODO: Surface CLI options (e.g., config override, diagnostics) and handle errors.

    caps::core::AppContext context;
    context.Initialize(config_path);

    caps::platform::macos::PlatformApp platform_app(context);
    platform_app.Initialize();
    platform_app.Run();
    platform_app.Shutdown();

    std::cout << "[macOS::Main] Exiting skeleton" << std::endl;
    return 0;
}
