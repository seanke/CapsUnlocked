#include "layer_controller.h"

#include <utility>

#include "core/mapping/mapping_engine.h"
#include "core/overlay/overlay_model.h"

namespace caps::core {

LayerController::LayerController(MappingEngine& mapping,
                                 OverlayModel& overlay)
    : mapping_(mapping), overlay_(overlay) {}

void LayerController::SetActionCallback(ActionCallback callback) {
    action_callback_ = std::move(callback);
}

void LayerController::OnCapsLockPressed() {
    layer_active_ = true;
    overlay_.Hide();
}

void LayerController::OnCapsLockReleased() {
    layer_active_ = false;
    overlay_.Hide();
}

bool LayerController::OnKeyEvent(const KeyEvent& event) {
    if (!layer_active_) {
        return false;
    }

    const auto mapping = mapping_.ResolveMapping(event.key);
    if (!mapping) {
        return true; // swallow unmapped keys while the layer is active
    }

    if (action_callback_) {
        action_callback_(*mapping, event.pressed);
    }
    return true;
}

void LayerController::OnDoubleTapCapsLock() {
    overlay_.Show();
}

bool LayerController::IsLayerActive() const {
    return layer_active_;
}

} // namespace caps::core
