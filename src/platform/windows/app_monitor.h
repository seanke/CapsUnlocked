#pragma once

#include <string>

namespace caps::platform::windows {

// Utility that reports the currently focused application name on Windows.
// Uses GetForegroundWindow and GetWindowTextW to identify the active window.
class AppMonitor {
public:
    std::string CurrentAppName() const;
};

} // namespace caps::platform::windows
