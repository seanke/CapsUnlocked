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
        std::vector<std::string> required_mods; // modifiers that must be held for this mapping
    };

    // Resolves a mapping considering currently active modifiers.
    // active_mods: set of currently pressed modifier keys (normalized)
    [[nodiscard]] std::optional<ResolvedMapping> ResolveMapping(
        const std::string& key,
        const std::string& app,
        const std::set<std::string>& active_mods = {}) const;
        
    // Check if a key is registered as a modifier
    [[nodiscard]] bool IsModifier(const std::string& key) const;
    
    // Get all registered modifiers
    [[nodiscard]] const std::set<std::string>& GetModifiers() const;
    
    struct MappingEntry {
        std::string app;
        std::string source;
        std::string target;
        std::vector<std::string> required_mods;
    };
    [[nodiscard]] std::vector<MappingEntry> EnumerateMappings() const;
    static std::string NormalizeAppToken(const std::string& app);

private:
    void RebuildTable();
    static std::string NormalizeToken(const std::string& key);

    const ConfigLoader& config_;
    // app -> list of mapping definitions (ordered by specificity: more modifiers first)
    std::unordered_map<std::string, std::vector<MappingDefinition>> resolved_;
    std::set<std::string> modifiers_;
};

} // namespace caps::core
