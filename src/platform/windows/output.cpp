#include "output.h"

// CapsUnlocked Windows adapter: placeholder for synthetic key emission, logging
// actions that will later translate into SendInput calls.

#include <iostream>

namespace caps::platform::windows {

void Output::Emit(const std::string& action) {
    std::cout << "[Windows::Output] Emitting action " << action << std::endl;
    // TODO: Translate `action` into INPUT structures and dispatch via SendInput.
}

} // namespace caps::platform::windows
