#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace caps::core {

class MappingEngine;

// Bitmask for modifier key states. Values chosen to match common representations.
enum class Modifiers : uint32_t {
    None = 0,
    Shift = 1 << 0,
    Control = 1 << 1,
    Alt = 1 << 2,    // Option on macOS
    Meta = 1 << 3,   // Windows/Super key on Windows, Command on macOS
};

inline Modifiers operator|(Modifiers a, Modifiers b) {
    return static_cast<Modifiers>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline Modifiers operator&(Modifiers a, Modifiers b) {
    return static_cast<Modifiers>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool HasModifier(Modifiers mods, Modifiers flag) {
    return (static_cast<uint32_t>(mods) & static_cast<uint32_t>(flag)) != 0;
}

// Lightweight struct that represents a key + press/release state as captured by hooks.
struct KeyEvent {
    std::string key;
    std::string app; // Normalized application identifier (empty uses fallback mappings).
    bool pressed{false};
    Modifiers modifiers{Modifiers::None}; // Active modifier keys (excluding CapsLock itself)
};

// Central coordinator that knows whether the Caps layer is active and which mappings apply.
class LayerController {
public:
    // Callback signature includes modifiers so that synthetic events can preserve them.
    using ActionCallback = std::function<void(const std::string& action, bool pressed, Modifiers modifiers)>;

    explicit LayerController(MappingEngine& mapping);

    void SetActionCallback(ActionCallback callback);

    void OnCapsLockPressed();
    void OnCapsLockReleased();
    // Returns true when the event was consumed by the layer (so hooks can swallow originals).
    bool OnKeyEvent(const KeyEvent& event);

    [[nodiscard]] bool IsLayerActive() const;

private:
    MappingEngine& mapping_;
    ActionCallback action_callback_;
    bool layer_active_{false};
};

} // namespace caps::core
