#pragma once

#include <string>
#include <utility>
#include <vector>

namespace caps::core {

class MappingEngine;

// Stores the data the overlay UI needs (ordered key/action rows + visibility flag).
class OverlayModel {
public:
    // Re-snapshots the mapping table; call this whenever the config changes.
    void BindMappings(const MappingEngine& engine);
    // Marks the overlay as visible so platform adapters can animate it in.
    void Show();
    // Marks the overlay hidden and lets platform adapters dismiss their UI.
    void Hide();
    // Returns a multi-line summary for quick logging or accessibility hooks.
    [[nodiscard]] std::string Describe() const;

private:
    std::vector<std::pair<std::string, std::string>> entries_;
    bool visible_{false};
};

} // namespace caps::core
