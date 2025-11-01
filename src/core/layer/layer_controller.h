#pragma once

#include <string>

namespace caps::core {

class ConfigLoader;
class MappingEngine;
class OverlayModel;

struct KeyEvent {
    std::string key;
    bool pressed{false};
};

class LayerController {
public:
    LayerController(ConfigLoader& config, MappingEngine& mapping, OverlayModel& overlay);

    void OnCapsLockPressed();
    void OnCapsLockReleased();
    void OnKeyEvent(const KeyEvent& event);
    void OnDoubleTapCapsLock();

    [[nodiscard]] bool IsLayerActive() const;

private:
    ConfigLoader& config_;
    MappingEngine& mapping_;
    OverlayModel& overlay_;
    bool layer_active_{false};
};

} // namespace caps::core
