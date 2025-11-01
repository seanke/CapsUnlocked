## Features

### CapsLock Layer Modifier
- **Description:** Hold `CapsLock` to activate a second keyboard layer across Windows 11 and modern macOS.
- **Build Notes:** Implement low-level keyboard hooks on both platforms (LLKHF on Windows, CGEvent tap on macOS) driven by shared logic.
- **Status:** planned

### Configurable Remaps
- **Description:** Layer behavior is defined by a config file; each key can be mapped to an alternate virtual key or shortcut.
- **Build Notes:** Reuse the existing `capsunlocked.ini` parser with per-platform keycode translation tables.
- **Status:** planned

### Unmapped Key Suppression
- **Description:** When CapsLock is held, keys without explicit mappings are swallowed (e.g. `CapsLock+g` emits nothing).
- **Build Notes:** Inject synthetic events only for configured mappings; block originals at the hook/tap.
- **Status:** planned

### Overlay on Double-Tap
- **Description:** Rapidly tapping CapsLock twice displays an overlay listing active mappings; pressing CapsLock again dismisses it.
- **Build Notes:** Windows overlays via layered window; macOS via transparent NSPanel or CoreAnimation overlay, sharing layout data.
- **Status:** planned

### Disable Native CapsLock Toggle
- **Description:** While the helper runs, the OS no longer toggles CapsLock state, preventing accidental caps changes.
- **Build Notes:** Consume CapsLock press/release in the hook/tap before they reach the system; reset LED state if needed.
- **Status:** planned
