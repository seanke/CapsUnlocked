#include "overlay_view.h"

// CapsUnlocked Windows adapter: stub overlay renderer that will eventually draw
// the mapping list; currently echoes overlay model descriptions.

#include "core/overlay/overlay_model.h"
#include "core/logging.h"

namespace caps::platform::windows {

OverlayView::OverlayView(core::OverlayModel& model) : model_(model) {
    core::logging::Info("[Windows::OverlayView] Constructed overlay view");
}

void OverlayView::Show() {
    core::logging::Info("[Windows::OverlayView] Show overlay -> " + model_.Describe());
    // TODO: Create or update a layered/transparent window to display mapping rows.
}

void OverlayView::Hide() {
    core::logging::Info("[Windows::OverlayView] Hide overlay");
    // TODO: Tear down or hide the overlay window and release resources.
}

} // namespace caps::platform::windows
