#include "layer_controller.h"

// CapsUnlocked core: controls CapsLock layer state, routing events between hooks,
// mapping engine, and overlay while currently emitting trace logs only.

#include <iostream>

#include "core/config/config_loader.h"
#include "core/mapping/mapping_engine.h"
#include "core/overlay/overlay_model.h"

namespace caps::core {

LayerController::LayerController(ConfigLoader& config,
                                 MappingEngine& mapping,
                                 OverlayModel& overlay)
    : config_(config), mapping_(mapping), overlay_(overlay) {
    std::cout << "[LayerController] Constructed layer controller" << std::endl;
    // TODO: Subscribe to config/mapping change notifications to refresh runtime state.
}

void LayerController::OnCapsLockPressed() {
    layer_active_ = true;
    std::cout << "[LayerController] CapsLock pressed -> layer active ("
              << config_.Describe() << ")" << std::endl;
    // TODO: Track activation time for double-tap detection and overlay toggling.
}

void LayerController::OnCapsLockReleased() {
    layer_active_ = false;
    std::cout << "[LayerController] CapsLock released -> layer inactive" << std::endl;
}

void LayerController::OnKeyEvent(const KeyEvent& event) {
    std::cout << "[LayerController] Handling key event " << event.key
              << " pressed=" << std::boolalpha << event.pressed << std::endl;
    if (layer_active_) {
        std::cout << "[LayerController] Active layer mapping -> "
                  << mapping_.ResolveMapping(event.key) << std::endl;
        // TODO: Request mapped action emission and optionally swallow originals.
    }
}

void LayerController::OnDoubleTapCapsLock() {
    std::cout << "[LayerController] Double tap CapsLock detected" << std::endl;
    overlay_.Show();
    // TODO: Toggle overlay visibility and manage dismissal on next CapsLock press.
}

bool LayerController::IsLayerActive() const {
    return layer_active_;
}

} // namespace caps::core
