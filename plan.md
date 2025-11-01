# Delivery Plan

This plan lists the major tasks required to take the current CapsUnlocked scaffold to a production-ready cross-platform application. Tasks are ordered to build on earlier work and include testing milestones.

## Phase 1 – Core Functionality
1. **Config Parsing**
   - Implement `ConfigLoader::Load/Reload` to read `capsunlocked.ini`, support comments, and normalize key tokens.
   - Add unit tests covering happy path and malformed configs.
2. **Mapping Engine**
   - Implement `MappingEngine` initialisation and lookup logic, including unmapped-key suppression behaviour.
   - Unit tests for mapping resolution, default fallbacks, and case sensitivity.
3. **Layer Controller Logic**
   - Implement activation state management, double-tap detection, and mapped-action dispatch.
   - Unit tests simulating CapsLock press/release sequences, double taps, and timing edge cases.
4. **Overlay Model**
   - Generate overlay rows from the mapping engine and manage visibility flags.
   - Unit tests verifying overlay content and state transitions.
5. **App Context Wiring**
   - Hook up change notifications so config reloads refresh mapping/overlay data.
   - Integration-style unit tests to ensure context initialises and propagates updates correctly.

## Phase 2 – macOS Adapter
6. **Event Tap & HID Listener**
   - Implement CGEvent tap setup, IOHID monitoring, and translation to `LayerController` events.
   - Manual/automated integration tests using macOS accessibility test harnesses if available.
7. **Synthetic Event Output**
   - Implement `macos::Output::Emit` with `CGEventCreateKeyboardEvent`.
   - Write integration tests that verify generated events (e.g., via accessibility APIs or mockable layer).
8. **Overlay View**
   - Build transparent overlay window (NSPanel or CoreAnimation) and bind to `OverlayModel`.
   - Add UI automation or snapshot tests if feasible; otherwise document manual QA checklist.
9. **Platform Lifecycle**
   - Flesh out `macos::PlatformApp` run-loop management, permission prompts, and shutdown.
   - End-to-end smoke test script invoking the binary, verifying overlay toggle and remaps.

## Phase 3 – Windows Adapter
10. **Keyboard Hook**
    - Implement `WH_KEYBOARD_LL` hook, translate events to layer controller, manage message loop threading.
    - Add integration tests using Windows input simulation or test harness; document manual steps.
11. **Synthetic Event Output**
    - Implement `SendInput` emission for mapped actions.
    - Tests verifying key injection (e.g., via a dedicated test window or Win32 hooks).
12. **Overlay Window**
    - Implement layered/transparent overlay showing mappings.
    - UI automation or manual test checklist to verify display and dismissal.
13. **Platform Lifecycle**
    - Complete `windows::PlatformApp` setup/shutdown, config reload handling, and tray/menu integration if desired.
    - End-to-end smoke test on Windows validating remaps and overlay.

## Phase 4 – Cross-Cutting Polish
14. **Config Reload & Hot Updates**
    - Add file watcher to reload config on change (optional but desirable).
    - Tests ensuring live update doesn’t break active layer.
15. **Error Handling & Logging**
    - Replace `std::cout` scaffolding with structured logging; ensure clear user messaging on permission issues.
16. **CLI & Diagnostics**
    - Implement command-line options for config path overrides, verbose logging, and debug info.
17. **Packaging**
    - Create build scripts/installers for macOS (bundle or notarized app) and Windows (installer or portable exe).
18. **Documentation & Samples**
    - Update `README`, `features.md`, and create sample configs; document overlay shortcuts.
19. **Final QA & Regression Tests**
    - Run full unit test suite, platform integration tests, and manual regression passes on supported OS versions.

Progress through phases sequentially, revisiting earlier layers only if shared abstractions need refinement. Unit testing focuses on the core first so platform adapters rely on already-verified behaviour.***
