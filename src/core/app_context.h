#pragma once

#include <string>

#include "core/config/config_loader.h"
#include "core/layer/layer_controller.h"
#include "core/mapping/mapping_engine.h"
#include "core/overlay/overlay_model.h"

namespace caps::core {

class AppContext {
public:
    AppContext();

    void Initialize(const std::string& config_path);

    ConfigLoader& Config();
    MappingEngine& Mapping();
    OverlayModel& Overlay();
    LayerController& Layer();

private:
    ConfigLoader config_loader_;
    MappingEngine mapping_engine_;
    OverlayModel overlay_model_;
    LayerController layer_controller_;
};

} // namespace caps::core
