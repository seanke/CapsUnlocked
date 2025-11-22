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

// Returns the mapped action if the layer defines one for the given app (with fallback). Otherwise std::nullopt.
std::optional<MappingEngine::ResolvedMapping> MappingEngine::ResolveMapping(const std::string& key,
                                                                            const std::string& app) const {
    if (key.empty()) {
        return std::nullopt;
    }

    const std::string normalized = NormalizeToken(key);
    const std::string normalized_app = NormalizeAppToken(app);

    // Prefer an app-specific mapping when available, otherwise fall back to "*".
    const auto by_app = resolved_.find(normalized_app);
    if (by_app != resolved_.end()) {
        const auto it = by_app->second.find(normalized);
        if (it != by_app->second.end()) {
            return ResolvedMapping{it->second, normalized_app};
        }
    }

    const auto fallback = resolved_.find("*");
    if (fallback != resolved_.end()) {
        const auto it = fallback->second.find(normalized);
        if (it != fallback->second.end()) {
            return ResolvedMapping{it->second, "*"};
        }
    }

    return std::nullopt;
}

// Exposes ordered rows for logging or debugging tooling.
std::vector<MappingEngine::MappingEntry> MappingEngine::EnumerateMappings() const {
    std::vector<MappingEntry> ordered;
    for (const auto& [app, table] : resolved_) {
        for (const auto& [source, target] : table) {
            ordered.push_back(MappingEntry{app, source, target});
        }
    }
    std::sort(ordered.begin(), ordered.end(),
              [](const auto& lhs, const auto& rhs) {
                  if (lhs.app == rhs.app) {
                      return lhs.source < rhs.source;
                  }
                  return lhs.app < rhs.app;
              });
    return ordered;
}

// Normalizes every key in the config into a hash table for O(1) lookups.
void MappingEngine::RebuildTable() {
    resolved_.clear();
    for (const auto& [app, table] : config_.Mappings()) {
        for (const auto& [source, target] : table) {
            // Each config entry collapses to a single uppercase token to avoid duplicate keys from case differences.
            resolved_[NormalizeAppToken(app)][NormalizeToken(source)] = target;
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
