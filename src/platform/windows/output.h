#pragma once

#include <string>

namespace caps::platform::windows {

class Output {
public:
    void Emit(const std::string& action);
};

} // namespace caps::platform::windows
