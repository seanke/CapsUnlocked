#include "app_context.h"

// CapsUnlocked core: central wiring point that stitches config, mapping, and
// layer controller services together for platform entry points.

#include "core/logging.h"

namespace caps::core {

AppContext::AppContext()
    : mapping_engine_(config_loader_),
      layer_controller_(mapping_engine_) {
    logging::Info("[AppContext] Context constructed");
}

// Platform main() calls this once after picking a config path to wire everything up.
void AppContext::Initialize(const std::string& config_path) {
    logging::Info("[AppContext] Initializing with config " + config_path);
    // Step 1: read the INI file so downstream services can see the new mappings.
    config_loader_.Load(config_path);
    // Step 2: ensure the mapping engine has fresh caches before it serves lookups.
    mapping_engine_.Initialize();
    mapping_engine_.UpdateFromConfig();
    // TODO: Wire config change notifications and persist context state.
}

ConfigLoader& AppContext::Config() {
    return config_loader_;
}

MappingEngine& AppContext::Mapping() {
    return mapping_engine_;
}

LayerController& AppContext::Layer() {
    return layer_controller_;
}

} // namespace caps::core
