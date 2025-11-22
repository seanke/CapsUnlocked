#pragma once

#include <string>

namespace caps::platform::macos {

// Small utility that reports the currently focused application name.
// Preference order: bundle identifier -> .app folder name -> executable name.
class AppMonitor {
public:
    std::string CurrentAppName() const;
};

} // namespace caps::platform::macos

