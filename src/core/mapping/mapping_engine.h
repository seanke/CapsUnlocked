#pragma once

#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/config/config_loader.h"

namespace caps::core {

// Lightweight view over ConfigLoader that exposes fast lookups and cached rows
// without re-reading the INI file.
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
        std::string modifier; // normalized modifier token used for this mapping ("*" for no modifier).
    };

    // Resolves a mapping with modifier keys support.
    // modifiers: set of currently held modifier keys (e.g., {"A"}, {"S"}, {"A", "S"})
    [[nodiscard]] std::optional<ResolvedMapping> ResolveMapping(const std::string& key,
                                                                const std::string& app,
                                                                const std::set<std::string>& modifiers = {}) const;
    struct MappingEntry {
        std::string app;
        std::string modifier;
        std::string source;
        std::string target;
    };
    [[nodiscard]] std::vector<MappingEntry> EnumerateMappings() const;

private:
    void RebuildTable();

    const ConfigLoader& config_;
    // app -> modifier -> key -> target
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>> resolved_;
};

} // namespace caps::core
