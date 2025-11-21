#ifdef _WIN32

#include <string>

// CapsUnlocked Windows executable entry point: bridges the core context with the
// Windows platform adapter, using wmain for wide-char argument handling.

#include "core/app_context.h"
#include "core/logging.h"
#include "platform/windows/platform_app.h"

int wmain(int argc, wchar_t* argv[]) {
    caps::core::logging::Info("[Windows::Main] Bootstrapping CapsUnlocked skeleton");

    std::string config_path = "capsunlocked.ini";
    if (argc > 1 && argv[1]) {
        std::wstring wide_path(argv[1]);
        // Convert the wide CLI argument into UTF-8 so ConfigLoader can read it.
        config_path.assign(wide_path.begin(), wide_path.end());
    }
    // TODO: Support command-line flags for config overrides and diagnostic modes.

    caps::core::AppContext context;
    context.Initialize(config_path);

    caps::platform::windows::PlatformApp platform_app(context);
    platform_app.Initialize();
    platform_app.Run();
    platform_app.Shutdown();

    caps::core::logging::Info("[Windows::Main] Exiting skeleton");
    return 0;
}

#endif // _WIN32
