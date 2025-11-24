# Agents

| Name | Scope | Responsibilities | Current Status |
| --- | --- | --- | --- |
| CoreAgent | `src/core/` | Owns configuration parsing, mapping resolution, overlay modeling, and layer control logic. Provides pure C++ services that platform adapters consume. | Scaffolding in place with TODO comments outlining parsing, mapping, overlay, and state-management work. |
| MacPlatformAgent | `src/platform/macos/` | Bridges macOS-specific APIs (CGEvent tap, IOHID, overlay rendering) into the core services; emits mapped events and displays the overlay. | Placeholder logging implementation; TODOs cover event tap installation, synthetic event emission, and overlay UI. |
| WinPlatformAgent | `src/platform/windows/` | Bridges Win32 APIs (low-level keyboard hook, SendInput, layered windows) into the core; coordinates message loop and overlay display. | Placeholder logging implementation; TODOs cover hook registration, SendInput wiring, and overlay window management. |
| Bootstrapper | `src/macos_main.cpp`, `src/windows_main.cpp` | Entry points that construct the core context, instantiate the platform agent, and drive the lifecycle (init → run → shutdown). | Logs lifecycle steps; TODOs call out future CLI/config enhancements. |

Each agent has intentionally thin interfaces so we can unit-test the core and keep platform-specific logic isolated. Consult `architecture.md` for the broader system design and the TODO comments in source files for implementation tasks.
