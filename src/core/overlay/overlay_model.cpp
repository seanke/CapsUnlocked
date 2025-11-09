#include "overlay_model.h"

#include <sstream>

#include "core/mapping/mapping_engine.h"

namespace caps::core {

void OverlayModel::BindMappings(const MappingEngine& engine) {
    entries_ = engine.EnumerateMappings();
}

void OverlayModel::Show() {
    visible_ = true;
}

void OverlayModel::Hide() {
    visible_ = false;
}

std::string OverlayModel::Describe() const {
    std::ostringstream output;
    output << (visible_ ? "overlay:visible" : "overlay:hidden");
    for (const auto& [source, target] : entries_) {
        output << "\n" << source << " -> " << target;
    }
    return output.str();
}

} // namespace caps::core
