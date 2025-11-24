# Architecture Overview

CapsUnlocked is split into a platform-neutral core and thin platform adapters for Windows and macOS. The current codebase contains scaffolding (logging only) plus TODO notes that describe the future implementation work.

## Core Library (`src/core/`)

| Component | Responsibility | Key TODOs |
| --- | --- | --- |
| `config/config_loader.{h,cpp}` | Load `capsunlocked.ini`, parse per-layer mappings, expose a human-readable summary. | Parse INI data, watch for changes, notify dependents. |
| `mapping/mapping_engine.{h,cpp}` | Hold the resolved mapping tables and answer lookup requests when the Caps layer is active. | Build efficient lookup structures, translate key tokens into actions. |
| `overlay/overlay_model.{h,cpp}` | Prepare overlay-friendly data (key → action rows) and track visibility state. | Maintain cached rows, notify platform views when shown/hidden. |
| `layer/layer_controller.{h,cpp}` | Manage CapsLock state, drive mapping lookups, coordinate overlay toggling, and swallow unmapped keys. | Handle double-tap detection, fire mapped actions, react to config changes. |
| `app_context.{h,cpp}` | Wire the four services together and provide accessors for platform code; orchestrate initialisation. | Propagate config reloads to mapping/overlay, persist shared state. |

The core is compiled into the `caps_core` static library and is intended to be unit-testable without OS hooks.

## Platform Adapters

### macOS (`src/platform/macos/`)

- `keyboard_hook.{h,cpp}` – will own the CGEvent tap plus IOHID listener so CapsLock events are still seen when the key is remapped to “No Action.”
- `output.{h,cpp}` – will convert mapped actions into `CGEvent` sequences.
- `overlay_view.{h,cpp}` – will render a transparent panel listing the active mappings.
- `platform_app.{h,cpp}` – orchestrates the adapter: installs hooks, enters the `CFRunLoop`, coordinates overlay lifecycle, and shuts everything down.

Each file currently logs its activity and contains explicit TODO comments describing the missing CGEvent/CFRunLoop work.

### Windows (`src/platform/windows/`)

- `keyboard_hook.{h,cpp}` – will install a `WH_KEYBOARD_LL` hook and translate Win32 messages into core events.
- `output.{h,cpp}` – will emit mapped actions via `SendInput`.
- `overlay_view.{h,cpp}` – will draw the overlay using a layered or transparent window.
- `platform_app.{h,cpp}` – will own the Win32 message loop, manage hook lifetime, and handle shutdown.

Like macOS, the files are currently stubs that log behaviour and list TODOs for the real Win32 implementation.

Both platform directories are compiled into `caps_platform` (platform-specific) static libraries that link against `caps_core`.

## Entry Points

- `src/macos_main.cpp` – Creates an `AppContext`, initialises it with the config path, constructs the macOS `PlatformApp`, and runs it.
- `src/windows_main.cpp` (guarded by `_WIN32`) – Equivalent bootstrapping for Windows via `wmain`.

Each entry point includes TODOs to expand CLI handling (config overrides, diagnostics) before handing control to the platform layer.

## Build & Run

- `CMakeLists.txt` defines `caps_core`, `caps_platform`, and the final `CapsUnlocked` executable for the active platform.
- `run.sh` configures and builds the project (default Release) and launches the resulting binary, making it easy to test the scaffold.

## Execution Flow (Skeleton)

1. Entry point initialises `AppContext`, which in turn instantiates the core services and logs diagnostics.
2. Platform `PlatformApp` installs a placeholder keyboard hook, then simulates CapsLock and mapped key events.
3. `LayerController` logs the active/inactive transitions and mapping resolutions.
4. Overlay components log show/hide behaviour.
5. Platform app shuts down, releasing stub resources.

As real implementations replace the logging stubs, the control flow will remain the same: platform adapters capture hardware events, forward them into the core, and use core responses to emit actions or update overlays.

Consult the TODO comments in each `.cpp` file for detailed implementation guidance. Once those TODOs are addressed, CapsUnlocked will deliver the feature set described in `features.md` (configurable Caps layer, overlay on double-tap, unmapped key suppression, etc.).
