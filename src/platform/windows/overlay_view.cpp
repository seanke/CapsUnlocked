#include "overlay_view.h"

// CapsUnlocked Windows adapter: stub overlay renderer that will eventually draw
// the mapping list; currently echoes overlay model descriptions.

#include <iostream>

#include "core/overlay/overlay_model.h"

namespace caps::platform::windows {

OverlayView::OverlayView(core::OverlayModel& model) : model_(model) {
    std::cout << "[Windows::OverlayView] Constructed overlay view" << std::endl;
}

void OverlayView::Show() {
    std::cout << "[Windows::OverlayView] Show overlay -> " << model_.Describe() << std::endl;
    // TODO: Create or update a layered/transparent window to display mapping rows.
}

void OverlayView::Hide() {
    std::cout << "[Windows::OverlayView] Hide overlay" << std::endl;
    // TODO: Tear down or hide the overlay window and release resources.
}

} // namespace caps::platform::windows
