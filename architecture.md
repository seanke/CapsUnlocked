## High-Level Architecture

### Core Concepts
- CapsUnlocked models CapsLock as a momentary modifier that activates a second keyboard layer.
- Platform-neutral services (config parsing, mapping resolution, overlay data modeling) live under `src/core/`.
- Platform-specific shims in `src/platform/<os>/` capture keyboard input, suppress native CapsLock behavior, and emit mapped events.
- The primary `main()` (per-platform target) wires core services to the OS adapters, keeping orchestration explicit and testable.

### Module Layout

#### Core Library (`src/core/`)
- `config/`: parses `capsunlocked.ini`, resolves textual identifiers to abstract key tokens.
- `mapping/`: owns layer state, key translation tables, and lookups.
- `overlay/`: formats the mapping list for presentation, independent of UI toolkit.
- `layer_controller/`: state machine for layer activation, double-tap detection, and key swallowing rules.

These units expose C++ interfaces that accept simple inputs (key tokens, timestamps) so they can be unit tested without OS hooks.

#### Platform Adapters (`src/platform/windows/`, `src/platform/macos/`)
- `keyboard_hook.cpp`: bridges OS events to `layer_controller`, translating keycodes to core tokens and suppressing native behavior.
- `output.cpp`: emits synthetic key events or overlay invocations via platform APIs.
- `overlay_view.cpp`: renders the overlay using native UI primitives but pulls data from the core overlay model.

Adapters implement thin wrappers around platform APIs. They depend only on the abstractions defined in the core library.

#### Entry Points (`src/windows_main.cpp`, `src/macos_main.cpp`)
- Create the shared `AppContext` (config loader, layer controller, overlay model).
- Initialize the matching platform adapter and register callbacks.
- Start the platform run loop (message pump on Windows, CFRunLoop on macOS).
- Provide a single orchestration location so platform-specific glue is easy to follow.

### Layer Activation Flow
1. Platform hook reports CapsLock press to `layer_controller`.
2. Controller raises the “layer active” flag and notifies the mapping engine.
3. Subsequent key events are converted to core tokens and routed to the mapping engine.
4. If a mapping exists, `output::emit_mapped_event` is invoked; otherwise the platform adapter swallows the key.
5. CapsLock release clears the layer flag and adapters resume pass-through delivery.
6. Double-tap detections from the controller trigger overlay show/hide calls via `overlay_view`.

### Component Breakdown

#### Config Loader (core)
- Parses `capsunlocked.ini` into normalized key identifiers.
- Emits abstract key tokens that remain stable across platforms.
- Provides query APIs for `(modifier, key)` lookups, enabling headless unit tests.

#### Event Hook (platform)
- Windows: low-level keyboard hook (`WH_KEYBOARD_LL`) converts Win32 virtual key codes to core tokens and forwards them.
- macOS: CGEvent tap plus IOHID listener converts CGKeyCodes to tokens and suppresses native CapsLock behavior.
- Both adapters defer double-tap timing to the shared controller, keeping stateful logic out of OS code.

#### Overlay Presenter
- Core overlay model prepares display rows (key + target + description).
- Platform view renders using native UI frameworks but consumes only model data, so the model can be unit tested.

#### HID Supplement (macOS)
- IOHIDManager listener tracks raw CapsLock usage so mappings still work when the OS remaps CapsLock to “No Action.”
- Feeds CapsLock state back into the shared layer flag without relying on system toggles.

### Threading & Run Loop
- Windows: message loop tied to the tray window pumps hook callbacks and overlay UI; adapters dispatch into core synchronously.
- macOS: CFRunLoop hosts CGEvent tap, HID callbacks, and overlay updates.
- Core components are thread-agnostic; adapters ensure calls occur on the appropriate thread and never block hook callbacks.

### Test Strategy
- Core modules (`config`, `mapping`, `layer_controller`, `overlay`) expose header-only interfaces and are compiled into a static library for unit tests.
- Platform adapters are exercised via integration tests or manual experiments, while thin wrappers minimize untestable logic.
- Use dependency injection in entry points (e.g., pass `std::unique_ptr<LayerController>` into adapter constructors) so mocks can be substituted in tests.

### Planned Extensions
- Configurable overlay themes and keyboard visualization.
- Multiple layers with different modifiers (e.g. CapsLock vs. CapsLock+Shift).
- Optional pass-through fallback where unmapped keys emit their original input.
