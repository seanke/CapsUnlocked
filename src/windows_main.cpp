#ifdef _WIN32

#include <string>
#include <string_view>

// CapsUnlocked Windows executable entry point: bridges the core context with the
// Windows platform adapter, using wmain for wide-char argument handling.

#include "core/app_context.h"
#include "core/logging.h"
#include "platform/windows/platform_app.h"

int wmain(int argc, wchar_t* argv[]) {
    caps::core::logging::Info("[Windows::Main] Bootstrapping CapsUnlocked skeleton");

    std::string config_path = "capsunlocked.ini";
    for (int i = 1; i < argc; ++i) {
        if (!argv[i]) {
            continue;
        }

        std::wstring wide_arg(argv[i]);
        std::string arg(wide_arg.begin(), wide_arg.end());
        std::string_view view(arg);

        if (view.rfind("--log-level=", 0) == 0) {
            const auto level_name = view.substr(std::string_view("--log-level=").size());
            if (const auto parsed = caps::core::logging::ParseLevel(level_name)) {
                caps::core::logging::SetLevel(*parsed);
                caps::core::logging::Info("[Windows::Main] Log level set to " + std::string(level_name));
            } else {
                caps::core::logging::Warn("[Windows::Main] Unknown log level '" + std::string(level_name) +
                                          "' (valid: debug, info, warn, error)");
            }
            continue;
        }

        // First non-flag argument is treated as config path override.
        config_path = arg;
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
