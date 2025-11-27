#include "mapping_engine.h"

#include <algorithm>
#include <cctype>

#include "core/string_utils.h"

namespace caps::core {

MappingEngine::MappingEngine(const ConfigLoader& config) : config_(config) {}

// Builds the initial lookup table. No-op if called more than once.
void MappingEngine::Initialize() {
    RebuildTable();
}

// Called after ConfigLoader reloads to refresh cached mappings atomically.
void MappingEngine::UpdateFromConfig() {
    RebuildTable();
}

// Returns the mapped action if the layer defines one for the given app and modifiers (with fallback). Otherwise std::nullopt.
std::optional<MappingEngine::ResolvedMapping> MappingEngine::ResolveMapping(const std::string& key,
                                                                            const std::string& app,
                                                                            const std::set<std::string>& modifiers) const {
    if (key.empty()) {
        return std::nullopt;
    }

    const std::string normalized_key = NormalizeKeyToken(key);
    const std::string normalized_app = caps::core::NormalizeAppToken(app);
    const std::string normalized_mod = caps::core::NormalizeModifierSet(modifiers);

    // Try app-specific mappings first
    const auto by_app = resolved_.find(normalized_app);
    if (by_app != resolved_.end()) {
        const auto by_mod = by_app->second.find(normalized_mod);
        if (by_mod != by_app->second.end()) {
            const auto it = by_mod->second.find(normalized_key);
            if (it != by_mod->second.end()) {
                return ResolvedMapping{it->second, normalized_app, normalized_mod};
            }
        }
    }

    // Fall back to "*" app with same modifier
    const auto fallback_app = resolved_.find("*");
    if (fallback_app != resolved_.end()) {
        const auto by_mod = fallback_app->second.find(normalized_mod);
        if (by_mod != fallback_app->second.end()) {
            const auto it = by_mod->second.find(normalized_key);
            if (it != by_mod->second.end()) {
                return ResolvedMapping{it->second, "*", normalized_mod};
            }
        }
    }

    return std::nullopt;
}

// Exposes ordered rows for logging or debugging tooling.
std::vector<MappingEngine::MappingEntry> MappingEngine::EnumerateMappings() const {
    std::vector<MappingEntry> ordered;
    for (const auto& [app, mod_table] : resolved_) {
        for (const auto& [modifier, key_table] : mod_table) {
            for (const auto& [source, target] : key_table) {
                ordered.push_back(MappingEntry{app, modifier, source, target});
            }
        }
    }
    std::sort(ordered.begin(), ordered.end(),
              [](const auto& lhs, const auto& rhs) {
                  if (lhs.app != rhs.app) return lhs.app < rhs.app;
                  if (lhs.modifier != rhs.modifier) return lhs.modifier < rhs.modifier;
                  return lhs.source < rhs.source;
              });
    return ordered;
}

// Normalizes every key in the config into a hash table for O(1) lookups.
void MappingEngine::RebuildTable() {
    resolved_.clear();
    for (const auto& [app, mod_table] : config_.Mappings()) {
        for (const auto& [modifier, key_table] : mod_table) {
            for (const auto& [source, target] : key_table) {
                // Each config entry collapses to a single uppercase token to avoid duplicate keys from case differences.
                resolved_[caps::core::NormalizeAppToken(app)][modifier][NormalizeKeyToken(source)] = target;
            }
        }
    }
}

} // namespace caps::core
