#include "output.h"

#include <iostream>

namespace caps::platform::windows {

void Output::Emit(const std::string& action, bool pressed) {
    std::cout << "[Windows::Output] Emitting action " << action
              << " pressed=" << std::boolalpha << pressed << std::endl;
    // TODO: Translate `action` into INPUT structures and dispatch via SendInput.
}

} // namespace caps::platform::windows
