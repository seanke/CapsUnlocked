#pragma once

#include <string>

#include "core/layer/layer_controller.h"

namespace caps::platform::macos {

// Translates abstract actions (e.g., "LEFT") into CGEvents and posts them.
class Output {
public:
    // `action` matches whatever MappingEngine::ResolveMapping returns (names or hex keycodes).
    // `pressed` mirrors the original key state so we emit down/up pairs.
    // `modifiers` contains the active modifier keys to preserve in the synthetic event.
    void Emit(const std::string& action, bool pressed, caps::core::Modifiers modifiers = caps::core::Modifiers::None);
};

} // namespace caps::platform::macos
