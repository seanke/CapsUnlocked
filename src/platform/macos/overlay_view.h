#pragma once

#include <string>

namespace caps::core {
class OverlayModel;
}

namespace caps::platform::macos {

// Placeholder for the future NSPanel overlay that will show mapping rows.
class OverlayView {
public:
    explicit OverlayView(core::OverlayModel& model);

    // Will eventually present an NSPanel/CoreAnimation overlay listing mappings.
    void Show();
    // Dismisses the overlay panel when CapsLock is pressed again.
    void Hide();

private:
    core::OverlayModel& model_;
};

} // namespace caps::platform::macos
