#include "overlay_model.h"

// CapsUnlocked core: overlay presentation model stub used by platform views to
// obtain layer mapping summaries.

#include <iostream>

#include "core/mapping/mapping_engine.h"

namespace caps::core {

void OverlayModel::BindMappings(const MappingEngine& engine) {
    std::cout << "[OverlayModel] Binding mappings: " << engine.ResolveMapping("preview") << std::endl;
    // TODO: Cache formatted layer entries for overlay display (key + action + hints).
}

void OverlayModel::Show() {
    std::cout << "[OverlayModel] Showing overlay" << std::endl;
    // TODO: Track visible state and notify platform views to render overlay contents.
}

void OverlayModel::Hide() {
    std::cout << "[OverlayModel] Hiding overlay" << std::endl;
    // TODO: Clear visible state and notify platform views to dismiss overlay.
}

std::string OverlayModel::Describe() const {
    std::cout << "[OverlayModel] Describing overlay contents" << std::endl;
    // TODO: Provide summary text (e.g., for logging or accessibility) of mappings shown.
    return "overlay:stub";
}

} // namespace caps::core
