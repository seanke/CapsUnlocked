#include "output.h"

// CapsUnlocked macOS adapter: stub sender for synthetic key events, currently
// logging the actions that will later be posted via CGEvent APIs.

#include <iostream>

namespace caps::platform::macos {

void Output::Emit(const std::string& action) {
    std::cout << "[macOS::Output] Emitting action " << action << std::endl;
    // TODO: Build CGEvent sequences corresponding to `action` and post them.
}

} // namespace caps::platform::macos
