#pragma once

#include <functional>
#include <string>

namespace caps::core {

class MappingEngine;

// Lightweight struct that represents a key + press/release state as captured by hooks.
struct KeyEvent {
    std::string key;
    std::string app; // Normalized application identifier (empty uses fallback mappings).
    bool pressed{false};
};

// Central coordinator that knows whether the Caps layer is active and which mappings apply.
class LayerController {
public:
    using ActionCallback = std::function<void(const std::string& action, bool pressed)>;

    explicit LayerController(MappingEngine& mapping);

    void SetActionCallback(ActionCallback callback);

    void OnCapsLockPressed();
    void OnCapsLockReleased();
    // Returns true when the event was consumed by the layer (so hooks can swallow originals).
    bool OnKeyEvent(const KeyEvent& event);

    [[nodiscard]] bool IsLayerActive() const;

private:
    MappingEngine& mapping_;
    ActionCallback action_callback_;
    bool layer_active_{false};
};

} // namespace caps::core
