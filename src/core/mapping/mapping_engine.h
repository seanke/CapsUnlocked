#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "core/config/config_loader.h"

namespace caps::core {

class MappingEngine {
public:
    explicit MappingEngine(const ConfigLoader& config);

    void Initialize();
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
