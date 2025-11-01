#pragma once

#include <string>

namespace caps::platform::macos {

class Output {
public:
    void Emit(const std::string& action);
};

} // namespace caps::platform::macos
