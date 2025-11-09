#pragma once

#include <string>

namespace caps::platform::macos {

class Output {
public:
    void Emit(const std::string& action, bool pressed);
};

} // namespace caps::platform::macos
