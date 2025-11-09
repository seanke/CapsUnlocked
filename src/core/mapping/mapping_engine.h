#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/config/config_loader.h"

namespace caps::core {

// Lightweight view over ConfigLoader that exposes fast lookups and cached rows
// for the overlay without re-reading the INI file.
class MappingEngine {
public:
    explicit MappingEngine(const ConfigLoader& config);

    // Initializes internal caches. Split so callers can create the object before
    // the config path is known.
    void Initialize();
    // Rebuilds the lookup table after ConfigLoader reloads.
    void UpdateFromConfig();

    [[nodiscard]] std::optional<std::string> ResolveMapping(const std::string& key) const;
    [[nodiscard]] std::vector<std::pair<std::string, std::string>> EnumerateMappings() const;

private:
    void RebuildTable();
    static std::string NormalizeToken(const std::string& key);

    const ConfigLoader& config_;
    std::unordered_map<std::string, std::string> resolved_;
};

} // namespace caps::core
