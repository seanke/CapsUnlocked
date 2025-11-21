#include "overlay_view.h"

// CapsUnlocked macOS adapter: placeholder overlay presenter that will ultimately
// render a transparent panel with mapping details.

#include "core/overlay/overlay_model.h"
#include "core/logging.h"

namespace caps::platform::macos {

OverlayView::OverlayView(core::OverlayModel& model) : model_(model) {
    core::logging::Info("[macOS::OverlayView] Constructed overlay view");
}

void OverlayView::Show() {
    core::logging::Info("[macOS::OverlayView] Show overlay -> " + model_.Describe());
    // TODO: Present semi-transparent overlay window/panel with mapping rows.
}

void OverlayView::Hide() {
    core::logging::Info("[macOS::OverlayView] Hide overlay");
    // TODO: Dismiss overlay window and clean up view/controller objects.
}

} // namespace caps::platform::macos
