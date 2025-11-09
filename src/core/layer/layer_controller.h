#pragma once

#include <functional>
#include <string>

namespace caps::core {

class MappingEngine;
class OverlayModel;

struct KeyEvent {
    std::string key;
    bool pressed{false};
};

class LayerController {
public:
    using ActionCallback = std::function<void(const std::string& action, bool pressed)>;

    LayerController(MappingEngine& mapping, OverlayModel& overlay);

    void SetActionCallback(ActionCallback callback);

    void OnCapsLockPressed();
    void OnCapsLockReleased();
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
