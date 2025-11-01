#pragma once

#include <string>

namespace caps::core {
class LayerController;
}

namespace caps::platform::windows {

class KeyboardHook {
public:
    void Install(core::LayerController& controller);
    void StartListening();
    void StopListening();

private:
    core::LayerController* controller_{nullptr};
};

} // namespace caps::platform::windows
