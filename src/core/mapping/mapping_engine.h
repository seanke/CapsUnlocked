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

    struct ResolvedMapping {
        std::string action;
        std::string app; // normalized app token that provided this mapping ("*" for fallback).
    };

    [[nodiscard]] std::optional<ResolvedMapping> ResolveMapping(const std::string& key,
                                                                const std::string& app) const;
    struct MappingEntry {
        std::string app;
        std::string source;
        std::string target;
    };
    [[nodiscard]] std::vector<MappingEntry> EnumerateMappings() const;
    static std::string NormalizeAppToken(const std::string& app);

private:
    void RebuildTable();
    static std::string NormalizeToken(const std::string& key);

    const ConfigLoader& config_;
    // app -> key -> target
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> resolved_;
};

} // namespace caps::core
