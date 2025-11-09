#pragma once

#include <string>

namespace caps::core {
class OverlayModel;
}

namespace caps::platform::windows {

// Placeholder for the layered window overlay that will mirror the macOS view.
class OverlayView {
public:
    explicit OverlayView(core::OverlayModel& model);

    void Show();
    void Hide();

private:
    core::OverlayModel& model_;
};

} // namespace caps::platform::windows
