#ifdef _WIN32

#include <iostream>
#include <string>

// CapsUnlocked Windows executable entry point: bridges the core context with the
// Windows platform adapter, using wmain for wide-char argument handling.

#include "core/app_context.h"
#include "platform/windows/platform_app.h"

int wmain(int argc, wchar_t* argv[]) {
    std::wcout << L"[Windows::Main] Bootstrapping CapsUnlocked skeleton" << std::endl;

    std::string config_path = "capsunlocked.ini";
    if (argc > 1 && argv[1]) {
        std::wstring wide_path(argv[1]);
        config_path.assign(wide_path.begin(), wide_path.end());
    }
    // TODO: Support command-line flags for config overrides and diagnostic modes.

    caps::core::AppContext context;
    context.Initialize(config_path);

    caps::platform::windows::PlatformApp platform_app(context);
    platform_app.Initialize();
    platform_app.Run();
    platform_app.Shutdown();

    std::wcout << L"[Windows::Main] Exiting skeleton" << std::endl;
    return 0;
}

#endif // _WIN32
