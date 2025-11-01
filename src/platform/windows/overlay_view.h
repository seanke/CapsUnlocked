#pragma once

#include <string>

namespace caps::core {
class OverlayModel;
}

namespace caps::platform::windows {

class OverlayView {
public:
    explicit OverlayView(core::OverlayModel& model);

    void Show();
    void Hide();

private:
    core::OverlayModel& model_;
};

} // namespace caps::platform::windows
