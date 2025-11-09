#pragma once

#include <string>

namespace caps::platform::windows {

class Output {
public:
    void Emit(const std::string& action, bool pressed);
};

} // namespace caps::platform::windows
