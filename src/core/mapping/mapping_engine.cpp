#include "mapping_engine.h"

#include <algorithm>
#include <cctype>

namespace caps::core {

MappingEngine::MappingEngine(const ConfigLoader& config) : config_(config) {}

void MappingEngine::Initialize() {
    RebuildTable();
}

void MappingEngine::UpdateFromConfig() {
    RebuildTable();
}

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

std::vector<std::pair<std::string, std::string>> MappingEngine::EnumerateMappings() const {
    std::vector<std::pair<std::string, std::string>> ordered;
    ordered.reserve(resolved_.size());
    for (const auto& [source, target] : resolved_) {
        ordered.emplace_back(source, target);
    }
    std::sort(ordered.begin(), ordered.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.first < rhs.first; });
    return ordered;
}

void MappingEngine::RebuildTable() {
    resolved_.clear();
    for (const auto& [source, target] : config_.Mappings()) {
        resolved_[NormalizeToken(source)] = target;
    }
}

std::string MappingEngine::NormalizeToken(const std::string& key) {
    std::string normalized;
    normalized.reserve(key.size());
    for (char ch : key) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    return normalized;
}

} // namespace caps::core
