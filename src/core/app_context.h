#pragma once

#include <string>

#include "core/config/config_loader.h"
#include "core/layer/layer_controller.h"
#include "core/mapping/mapping_engine.h"

namespace caps::core {

// Owns the lifetime of all core services so platform adapters can grab references
// without worrying about construction order.
class AppContext {
public:
    AppContext();

    // Loads the config, initializes dependent services, and wires them together.
    void Initialize(const std::string& config_path);

    ConfigLoader& Config();
    MappingEngine& Mapping();
    LayerController& Layer();

private:
    ConfigLoader config_loader_;
    MappingEngine mapping_engine_;
    LayerController layer_controller_;
};

} // namespace caps::core
