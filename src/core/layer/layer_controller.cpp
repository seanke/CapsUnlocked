#include "layer_controller.h"

#include <utility>
#include <sstream>
#include <cctype>

#include "core/mapping/mapping_engine.h"
#include "core/logging.h"

namespace caps::core {

namespace {

std::string FormatAppForLog(const std::string& app) {
    if (app.empty()) {
        return "*";
    }
    std::string formatted;
    formatted.reserve(app.size());
    for (char ch : app) {
        if (std::isalnum(static_cast<unsigned char>(ch))) {
            formatted.push_back(ch);
        }
    }
    if (formatted.empty()) {
        return "*";
    }
    return formatted;
}

} // namespace

LayerController::LayerController(MappingEngine& mapping)
    : mapping_(mapping) {}

// Platform adapters provide a callback that emits mapped actions when the layer is active.
void LayerController::SetActionCallback(ActionCallback callback) {
    action_callback_ = std::move(callback);
}

// Called whenever CapsLock is held down; activates the layer.
void LayerController::OnCapsLockPressed() {
    layer_active_ = true;
}

// Called when CapsLock is released; deactivates the layer.
void LayerController::OnCapsLockReleased() {
    layer_active_ = false;
}

// Routes key events through the mapping table and fires the synthetic action callback.
bool LayerController::OnKeyEvent(const KeyEvent& event) {
    if (!layer_active_) {
        return false;
    }

    const auto mapping = mapping_.ResolveMapping(event.key, event.app);
    if (event.pressed) {
        if (mapping) {
            std::ostringstream msg;
            const std::string resolved_app = mapping->app;
            const std::string map_app =
                resolved_app == "*" ? std::string("*") : FormatAppForLog(event.app.empty() ? resolved_app : event.app);
            msg << "Caps-held key " << event.key << " mapped to " << mapping->action
                << " (map=" << map_app << ")";
            logging::Debug(msg.str());
        } else {
            std::ostringstream msg;
            msg << "Caps-held key " << event.key << " has no mapping"
                ;
            logging::Debug(msg.str());
        }
    }

    if (!mapping) {
        return true; // swallow unmapped keys while the layer is active
    }

    if (action_callback_) {
        // Notify the platform adapter so it can emit synthetic events immediately.
        // Pass through the modifier state so that combinations like Ctrl+mapped-key work.
        action_callback_(mapping->action, event.pressed, event.modifiers);
    }
    return true;
}

bool LayerController::IsLayerActive() const {
    return layer_active_;
}

} // namespace caps::core
