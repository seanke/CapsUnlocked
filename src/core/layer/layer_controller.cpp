#include "layer_controller.h"

#include <utility>
#include <sstream>

#include "core/mapping/mapping_engine.h"
#include "core/overlay/overlay_model.h"
#include "core/logging.h"

namespace caps::core {

LayerController::LayerController(MappingEngine& mapping,
                                 OverlayModel& overlay)
    : mapping_(mapping), overlay_(overlay) {}

// Platform adapters provide a callback that emits mapped actions when the layer is active.
void LayerController::SetActionCallback(ActionCallback callback) {
    action_callback_ = std::move(callback);
}

// Called whenever CapsLock is held down; activates the layer and hides the overlay if present.
void LayerController::OnCapsLockPressed() {
    layer_active_ = true;
    overlay_.Hide();
}

// Called when CapsLock is released; deactivates the layer and dismisses overlays.
void LayerController::OnCapsLockReleased() {
    layer_active_ = false;
    overlay_.Hide();
}

// Routes key events through the mapping table and fires the synthetic action callback.
bool LayerController::OnKeyEvent(const KeyEvent& event) {
    if (!layer_active_) {
        return false;
    }

    const std::string normalized_app = MappingEngine::NormalizeAppToken(event.app);
    const auto mapping = mapping_.ResolveMapping(event.key, event.app);
    if (event.pressed) {
        if (mapping) {
            std::ostringstream msg;
            const std::string resolved_app = mapping->app;
            const std::string raw_app = event.app.empty() ? "*" : event.app;
            msg << "Caps-held key " << event.key << " mapped to " << mapping->action
                << " (app=" << raw_app
                << ", config_app=" << resolved_app
                << ")";
            logging::Debug(msg.str());
        } else {
            std::ostringstream msg;
            msg << "Caps-held key " << event.key << " has no mapping"
                << " (app=" << (event.app.empty() ? "*" : event.app)
                << ", config_app=" << normalized_app
                << ")";
            logging::Debug(msg.str());
        }
    }

    if (!mapping) {
        return true; // swallow unmapped keys while the layer is active
    }

    if (action_callback_) {
        // Notify the platform adapter so it can emit synthetic events immediately.
        action_callback_(mapping->action, event.pressed);
    }
    return true;
}

// Double-tapping CapsLock is interpreted as a request to show the reference overlay.
void LayerController::OnDoubleTapCapsLock() {
    overlay_.Show();
}

bool LayerController::IsLayerActive() const {
    return layer_active_;
}

} // namespace caps::core
