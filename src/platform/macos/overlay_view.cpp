#include "overlay_view.h"

// CapsUnlocked macOS adapter: placeholder overlay presenter that will ultimately
// render a transparent panel with mapping details.

#include <iostream>

#include "core/overlay/overlay_model.h"

namespace caps::platform::macos {

OverlayView::OverlayView(core::OverlayModel& model) : model_(model) {
    std::cout << "[macOS::OverlayView] Constructed overlay view" << std::endl;
}

void OverlayView::Show() {
    std::cout << "[macOS::OverlayView] Show overlay -> " << model_.Describe() << std::endl;
    // TODO: Present semi-transparent overlay window/panel with mapping rows.
}

void OverlayView::Hide() {
    std::cout << "[macOS::OverlayView] Hide overlay" << std::endl;
    // TODO: Dismiss overlay window and clean up view/controller objects.
}

} // namespace caps::platform::macos
