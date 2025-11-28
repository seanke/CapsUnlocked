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
// When multiple mappings exist for the same source key, the one with the most matching modifiers wins.
std::optional<MappingEngine::ResolvedMapping> MappingEngine::ResolveMapping(
    const std::string& key,
    const std::string& app,
    const std::set<std::string>& active_mods) const {
    if (key.empty()) {
        return std::nullopt;
    }

    const std::string normalized = NormalizeToken(key);
    const std::string normalized_app = NormalizeAppToken(app);

    // Helper to find best matching mapping from a definitions list
    auto find_best_match = [&](const std::vector<MappingDefinition>& definitions) 
        -> std::optional<const MappingDefinition*> {
        const MappingDefinition* best = nullptr;
        size_t best_mod_count = 0;
        
        for (const auto& def : definitions) {
            if (def.source != normalized) {
                continue;
            }
            
            // Check if all required modifiers are active
            bool all_mods_active = true;
            for (const auto& mod : def.required_mods) {
                if (active_mods.count(mod) == 0) {
                    all_mods_active = false;
                    break;
                }
            }
            
            if (!all_mods_active) {
                continue;
            }
            
            // Prefer mappings with more modifiers (more specific).
            // When counts are equal, keep the first match found (config file order determines priority).
            if (best == nullptr || def.required_mods.size() > best_mod_count) {
                best = &def;
                best_mod_count = def.required_mods.size();
            }
        }
        
        if (best) {
            return best;
        }
        return std::nullopt;
    };

    // Prefer an app-specific mapping when available, otherwise fall back to "*".
    const auto by_app = resolved_.find(normalized_app);
    if (by_app != resolved_.end()) {
        if (auto match = find_best_match(by_app->second)) {
            return ResolvedMapping{(*match)->target, normalized_app, (*match)->required_mods};
        }
    }

    const auto fallback = resolved_.find("*");
    if (fallback != resolved_.end()) {
        if (auto match = find_best_match(fallback->second)) {
            return ResolvedMapping{(*match)->target, "*", (*match)->required_mods};
        }
    }

    return std::nullopt;
}

// Check if a key is registered as a modifier
bool MappingEngine::IsModifier(const std::string& key) const {
    return modifiers_.count(NormalizeToken(key)) > 0;
}

// Get all registered modifiers
const std::set<std::string>& MappingEngine::GetModifiers() const {
    return modifiers_;
}

// Exposes ordered rows for logging or debugging tooling.
std::vector<MappingEngine::MappingEntry> MappingEngine::EnumerateMappings() const {
    std::vector<MappingEntry> ordered;
    for (const auto& [app, definitions] : resolved_) {
        for (const auto& def : definitions) {
            ordered.push_back(MappingEntry{app, def.source, def.target, def.required_mods});
        }
    }
    std::sort(ordered.begin(), ordered.end(),
              [](const auto& lhs, const auto& rhs) {
                  if (lhs.app == rhs.app) {
                      if (lhs.required_mods.size() == rhs.required_mods.size()) {
                          return lhs.source < rhs.source;
                      }
                      // More modifiers first (more specific)
                      return lhs.required_mods.size() > rhs.required_mods.size();
                  }
                  return lhs.app < rhs.app;
              });
    return ordered;
}

// Normalizes every key in the config into a hash table for O(1) lookups.
void MappingEngine::RebuildTable() {
    resolved_.clear();
    modifiers_ = config_.Modifiers();
    
    for (const auto& [app, definitions] : config_.Mappings()) {
        auto& app_mappings = resolved_[NormalizeAppToken(app)];
        for (const auto& def : definitions) {
            MappingDefinition normalized_def;
            normalized_def.source = NormalizeToken(def.source);
            normalized_def.target = def.target;
            normalized_def.required_mods = def.required_mods;
            app_mappings.push_back(std::move(normalized_def));
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
