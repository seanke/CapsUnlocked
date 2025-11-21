#include "output.h"

#include "core/logging.h"

#include <sstream>

namespace caps::platform::windows {

// Logs the mapped action for now; real implementation will use SendInput.
void Output::Emit(const std::string& action, bool pressed) {
    std::ostringstream msg;
    msg << "[Windows::Output] Emitting action " << action << " pressed=" << std::boolalpha << pressed;
    core::logging::Info(msg.str());
    // TODO: Translate `action` into INPUT structures and dispatch via SendInput.
}

} // namespace caps::platform::windows
