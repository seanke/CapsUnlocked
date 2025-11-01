#include "config_loader.h"

// CapsUnlocked core: placeholder configuration loader that logs file operations
// until real parsing is implemented.

#include <iostream>

namespace caps::core {

void ConfigLoader::Load(const std::string& path) {
    std::cout << "[ConfigLoader] Loading config from " << path << std::endl;
    // TODO: Read `path`, parse INI-style mappings, and populate mapping tables.
}

void ConfigLoader::Reload() {
    std::cout << "[ConfigLoader] Reloading config" << std::endl;
    // TODO: Re-read the last known config path and notify dependents of changes.
}

std::string ConfigLoader::Describe() const {
    std::cout << "[ConfigLoader] Describing current configuration" << std::endl;
    // TODO: Return a human-readable summary of current mappings for debugging/UI.
    return "config:stub";
}

} // namespace caps::core
