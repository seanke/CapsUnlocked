#include "overlay_model.h"

#include <sstream>

#include "core/mapping/mapping_engine.h"

namespace caps::core {

void OverlayModel::BindMappings(const MappingEngine& engine) {
    entries_.clear();
    for (const auto& entry : engine.EnumerateMappings()) {
        entries_.emplace_back(entry.app, entry.source, entry.target);
    }
}

// Called when CapsLock is double-tapped so the UI knows to show the reference sheet.
void OverlayModel::Show() {
    visible_ = true;
}

// Lets the platform view dismiss the overlay when CapsLock is pressed again.
void OverlayModel::Hide() {
    visible_ = false;
}

// Simple textual dump that is handy while developing without a UI.
std::string OverlayModel::Describe() const {
    std::ostringstream output;
    output << (visible_ ? "overlay:visible" : "overlay:hidden");
    for (const auto& [app, source, target] : entries_) {
        output << "\n[" << app << "] " << source << " -> " << target;
    }
    return output.str();
}

} // namespace caps::core
