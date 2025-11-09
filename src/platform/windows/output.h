#pragma once

#include <string>

namespace caps::platform::windows {

// Thin wrapper that will eventually turn mapped actions into SendInput calls.
class Output {
public:
    void Emit(const std::string& action, bool pressed);
};

} // namespace caps::platform::windows
