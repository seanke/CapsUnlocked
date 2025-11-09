#pragma once

#include <functional>
#include <string>

namespace caps::core {

class MappingEngine;
class OverlayModel;

// Lightweight struct that represents a key + press/release state as captured by hooks.
struct KeyEvent {
    std::string key;
    bool pressed{false};
};

// Central coordinator that knows whether the Caps layer is active, which mappings apply,
// and when to trigger overlay visibility or mapped output callbacks.
class LayerController {
public:
    using ActionCallback = std::function<void(const std::string& action, bool pressed)>;

    LayerController(MappingEngine& mapping, OverlayModel& overlay);

    void SetActionCallback(ActionCallback callback);

    void OnCapsLockPressed();
    void OnCapsLockReleased();
    // Returns true when the event was consumed by the layer (so hooks can swallow originals).
    bool OnKeyEvent(const KeyEvent& event);
    void OnDoubleTapCapsLock();

    [[nodiscard]] bool IsLayerActive() const;

private:
    MappingEngine& mapping_;
    OverlayModel& overlay_;
    ActionCallback action_callback_;
    bool layer_active_{false};
};

} // namespace caps::core
