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

std::string NormalizeKey(const std::string& key) {
    std::string normalized;
    normalized.reserve(key.size());
    for (char ch : key) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        }
    }
    return normalized;
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
    // Clear all active modifiers when layer is deactivated
    active_modifiers_.clear();
}

// Routes key events through the mapping table and fires the synthetic action callback.
bool LayerController::OnKeyEvent(const KeyEvent& event) {
    if (!layer_active_) {
        return false;
    }

    const std::string normalized_key = NormalizeKey(event.key);
    
    // Check if this key is a modifier
    if (mapping_.IsModifier(normalized_key)) {
        if (event.pressed) {
            active_modifiers_.insert(normalized_key);
            std::ostringstream msg;
            msg << "Modifier " << normalized_key << " pressed (active: " << active_modifiers_.size() << ")";
            logging::Debug(msg.str());
        } else {
            active_modifiers_.erase(normalized_key);
            std::ostringstream msg;
            msg << "Modifier " << normalized_key << " released (active: " << active_modifiers_.size() << ")";
            logging::Debug(msg.str());
        }
        // Swallow modifier key events - they should not pass through
        return true;
    }

    const auto mapping_result = mapping_.ResolveMapping(event.key, event.app, active_modifiers_);
    if (event.pressed) {
        if (mapping_result) {
            std::ostringstream msg;
            const std::string resolved_app = mapping_result->app;
            const std::string map_app =
                resolved_app == "*" ? std::string("*") : FormatAppForLog(event.app.empty() ? resolved_app : event.app);
            msg << "Caps-held key " << event.key << " mapped to " << mapping_result->action;
            if (!mapping_result->required_mods.empty()) {
                msg << " (mods: ";
                for (size_t i = 0; i < mapping_result->required_mods.size(); ++i) {
                    if (i > 0) msg << "+";
                    msg << mapping_result->required_mods[i];
                }
                msg << ")";
            }
            msg << " (map=" << map_app << ")";
            logging::Debug(msg.str());
        } else {
            std::ostringstream msg;
            msg << "Caps-held key " << event.key << " has no mapping";
            if (!active_modifiers_.empty()) {
                msg << " (active mods: ";
                bool first = true;
                for (const auto& mod : active_modifiers_) {
                    if (!first) msg << "+";
                    msg << mod;
                    first = false;
                }
                msg << ")";
            }
            logging::Debug(msg.str());
        }
    }

    if (!mapping_result) {
        return true; // swallow unmapped keys while the layer is active
    }

    if (action_callback_) {
        // Notify the platform adapter so it can emit synthetic events immediately.
        action_callback_(mapping_result->action, event.pressed);
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
