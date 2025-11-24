#pragma once

#include <string>

namespace caps::platform::windows {

// Translates abstract actions (e.g., "LEFT") into SendInput keyboard events.
class Output {
public:
    // `action` matches whatever MappingEngine::ResolveMapping returns (names or hex keycodes).
    // `pressed` mirrors the original key state so we emit down/up pairs.
    void Emit(const std::string& action, bool pressed);
};

} // namespace caps::platform::windows
