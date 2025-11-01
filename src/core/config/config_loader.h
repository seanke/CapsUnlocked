#pragma once

#include <string>

namespace caps::core {

class ConfigLoader {
public:
    void Load(const std::string& path);
    void Reload();
    [[nodiscard]] std::string Describe() const;
};

} // namespace caps::core
