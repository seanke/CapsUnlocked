#include "mapping_engine.h"

#include <algorithm>
#include <cctype>

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

// Converts a set of modifier keys to a normalized string like "A+S"
std::string MappingEngine::NormalizeModifierSet(const std::set<std::string>& modifiers) {
    if (modifiers.empty()) {
        return "*";
    }

    std::vector<std::string> sorted_mods;
    for (const auto& mod : modifiers) {
        std::string normalized;
        for (char ch : mod) {
            if (!std::isspace(static_cast<unsigned char>(ch))) {
                normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            }
        }
        if (!normalized.empty() && normalized != "*") {
            sorted_mods.push_back(normalized);
        }
    }

    if (sorted_mods.empty()) {
        return "*";
    }

    std::sort(sorted_mods.begin(), sorted_mods.end());
    sorted_mods.erase(std::unique(sorted_mods.begin(), sorted_mods.end()), sorted_mods.end());

    std::string result = sorted_mods[0];
    for (size_t i = 1; i < sorted_mods.size(); ++i) {
        result += '+';
        result += sorted_mods[i];
    }
    return result;
}

// Returns the mapped action if the layer defines one for the given app and modifiers (with fallback). Otherwise std::nullopt.
std::optional<MappingEngine::ResolvedMapping> MappingEngine::ResolveMapping(const std::string& key,
                                                                            const std::string& app,
                                                                            const std::set<std::string>& modifiers) const {
    if (key.empty()) {
        return std::nullopt;
    }

    const std::string normalized_key = NormalizeToken(key);
    const std::string normalized_app = NormalizeAppToken(app);
    const std::string normalized_mod = NormalizeModifierSet(modifiers);

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
                resolved_[NormalizeAppToken(app)][modifier][NormalizeToken(source)] = target;
            }
        }
    }
}

// Normalizes arbitrary key tokens so config entries can be matched case-insensitively.
std::string MappingEngine::NormalizeToken(const std::string& key) {
    std::string normalized;
    normalized.reserve(key.size());
    for (char ch : key) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            // Trim all whitespace so config entries like "h = left" still work.
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    return normalized;
}

std::string MappingEngine::NormalizeAppToken(const std::string& app) {
    if (app.empty()) {
        return "*";
    }

    std::string normalized;
    normalized.reserve(app.size());
    for (char ch : app) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    if (normalized.empty()) {
        return "*";
    }
    return normalized;
}

} // namespace caps::core
