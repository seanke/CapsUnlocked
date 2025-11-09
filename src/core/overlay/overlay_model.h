#pragma once

#include <string>
#include <utility>
#include <vector>

namespace caps::core {

class MappingEngine;

class OverlayModel {
public:
    void BindMappings(const MappingEngine& engine);
    void Show();
    void Hide();
    [[nodiscard]] std::string Describe() const;

private:
    std::vector<std::pair<std::string, std::string>> entries_;
    bool visible_{false};
};

} // namespace caps::core
