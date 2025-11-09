#pragma once

#include <cstdint>

namespace caps::platform::macos {

// Stored inside kCGEventSourceUserData so the event tap can detect events we emitted
// ourselves and avoid swallowing them a second time. Using a magic constant instead of
// pointer comparisons keeps the logic simple and works across processes.
inline constexpr int64_t kSyntheticEventTag = 0x43415053; // 'CAPS'

} // namespace caps::platform::macos
