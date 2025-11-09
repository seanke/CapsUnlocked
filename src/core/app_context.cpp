#include "app_context.h"

// CapsUnlocked core: central wiring point that stitches config, mapping, overlay,
// and layer controller services together for platform entry points.

#include "app_context.h"

// CapsUnlocked core: central wiring point that stitches config, mapping, overlay,
// and layer controller services together for platform entry points.

#include <iostream>

namespace caps::core {

AppContext::AppContext()
    : mapping_engine_(config_loader_),
      layer_controller_(mapping_engine_, overlay_model_) {
    std::cout << "[AppContext] Context constructed" << std::endl;
}

void AppContext::Initialize(const std::string& config_path) {
    std::cout << "[AppContext] Initializing with config " << config_path << std::endl;
    config_loader_.Load(config_path);
    mapping_engine_.Initialize();
    mapping_engine_.UpdateFromConfig();
    overlay_model_.BindMappings(mapping_engine_);
    // TODO: Wire config change notifications to mapping/overlay and persist context state.
}

ConfigLoader& AppContext::Config() {
    return config_loader_;
}

MappingEngine& AppContext::Mapping() {
    return mapping_engine_;
}

OverlayModel& AppContext::Overlay() {
    return overlay_model_;
}

LayerController& AppContext::Layer() {
    return layer_controller_;
}

} // namespace caps::core
