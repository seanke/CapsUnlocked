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

// Returns the mapped action if the layer defines one. Otherwise std::nullopt.
std::optional<std::string> MappingEngine::ResolveMapping(const std::string& key) const {
    if (key.empty()) {
        return std::nullopt;
    }

    const std::string normalized = NormalizeToken(key);
    const auto it = resolved_.find(normalized);
    if (it == resolved_.end()) {
        return std::nullopt;
    }
    return it->second;
}

// Exposes ordered rows for overlays, logging, or debugging tooling.
std::vector<std::pair<std::string, std::string>> MappingEngine::EnumerateMappings() const {
    std::vector<std::pair<std::string, std::string>> ordered;
    ordered.reserve(resolved_.size());
    for (const auto& [source, target] : resolved_) {
        // Keep the original key token alongside its resolved action for UI/diagnostics.
        ordered.emplace_back(source, target);
    }
    std::sort(ordered.begin(), ordered.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
    return ordered;
}

// Normalizes every key in the config into a hash table for O(1) lookups.
void MappingEngine::RebuildTable() {
    resolved_.clear();
    for (const auto& [source, target] : config_.Mappings()) {
        // Each config entry collapses to a single uppercase token to avoid duplicate keys from case differences.
        resolved_[NormalizeToken(source)] = target;
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

} // namespace caps::core
