#pragma once

#include <string>

namespace caps::core {

class MappingEngine {
public:
    void Initialize();
    void UpdateFromConfig();
    [[nodiscard]] std::string ResolveMapping(const std::string& key) const;
};

} // namespace caps::core
