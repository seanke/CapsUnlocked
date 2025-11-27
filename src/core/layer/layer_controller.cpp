#include "layer_controller.h"

#include <utility>
#include <sstream>
#include <cctype>

#include "core/mapping/mapping_engine.h"
#include "core/logging.h"
#include "core/string_utils.h"

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
    active_modifiers_.clear(); // Reset modifiers when layer is activated
}

// Called when CapsLock is released; deactivates the layer.
void LayerController::OnCapsLockReleased() {
    layer_active_ = false;
    active_modifiers_.clear(); // Clear modifiers when layer is deactivated
}

// Check if a key is a layer modifier that changes mapping lookup
bool LayerController::IsLayerModifier(const std::string& key) {
    const std::string normalized = NormalizeKeyToken(key);
    // A and S are the layer modifiers for navigation and selection
    return normalized == "A" || normalized == "S";
}

// Routes key events through the mapping table and fires the synthetic action callback.
bool LayerController::OnKeyEvent(const KeyEvent& event) {
    if (!layer_active_) {
        return false;
    }

    const std::string normalized_key = NormalizeKeyToken(event.key);

    // Track layer modifier keys (A, S) separately
    if (IsLayerModifier(event.key)) {
        if (event.pressed) {
            active_modifiers_.insert(normalized_key);
        } else {
            active_modifiers_.erase(normalized_key);
        }
        // Swallow modifier key events while layer is active
        return true;
    }

    const auto mapping = mapping_.ResolveMapping(event.key, event.app, active_modifiers_);
    if (event.pressed) {
        if (mapping) {
            std::ostringstream msg;
            const std::string resolved_app = mapping->app;
            const std::string map_app =
                resolved_app == "*" ? std::string("*") : FormatAppForLog(event.app.empty() ? resolved_app : event.app);
            msg << "Caps-held key " << event.key << " mapped to " << mapping->action
                << " (app=" << map_app << ", mod=" << mapping->modifier << ")";
            logging::Debug(msg.str());
        } else {
            std::ostringstream msg;
            msg << "Caps-held key " << event.key << " has no mapping";
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

bool LayerController::IsLayerActive() const {
    return layer_active_;
}

const std::set<std::string>& LayerController::GetActiveModifiers() const {
    return active_modifiers_;
}

} // namespace caps::core
