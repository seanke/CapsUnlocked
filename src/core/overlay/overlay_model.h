#pragma once

#include <string>

namespace caps::core {

class MappingEngine;

class OverlayModel {
public:
    void BindMappings(const MappingEngine& engine);
    void Show();
    void Hide();
    [[nodiscard]] std::string Describe() const;
};

} // namespace caps::core
