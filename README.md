# CapsUnlocked (Windows 11 & macOS)

A small keyboard layer tool that turns `CapsLock` into a momentary modifier. While held, selected keys are remapped based on a config file. By default, `h j k l` become the arrow keys. On Windows it lives in the system tray with a right-click Exit menu; on macOS it runs as a background console app.

## Features
- Hold `CapsLock` to activate the layer
- Remap keys via `capsunlocked.ini` next to the executable
- Default mapping: `h=Left`, `j=Down`, `k=Up`, `l=Right`
- Uses a low-level keyboard hook and `SendInput`

## Build
Prereqs: CMake 3.16+, a C++17 toolchain for your platform.

### Windows (MSVC)
```powershell
# from the repo root
cmake -S . -B build -A x64
cmake --build build --config Release
```
The resulting binary is at `build/Release/CapsUnlocked.exe` (or under your chosen config).

### macOS (Xcode or Ninja)
```bash
cmake -S . -B build
cmake --build build --config Release
```
The resulting executable is at `build/Release/CapsUnlocked` (or `build/CapsUnlocked` if your generator does not use configs).

## Run
- **Windows:** Launch the exe. A tray icon appears; right-click it and choose `Exit` to close. To intercept keystrokes for elevated apps (run as Administrator), run CapsUnlocked elevated because of Windows UIPI.
- **macOS:** Run the built binary from a terminal (e.g. `./build/Release/CapsUnlocked`). It logs a startup message and keeps running until you press `Ctrl+C`.

On first run CapsUnlocked writes a default `capsunlocked.ini` next to the executable if it can. If not, it falls back to the default mapping internally.

## Config
Config file path: `./capsunlocked.ini` (same folder as the executable)

Format:
```
# Lines like source=target
# Supported tokens: single characters (e.g. h, a) and common names (Left, Right, Up, Down, Enter, Esc, Tab, Space, Backspace, Delete, Home, End, PageUp, PageDown), or hex key codes (virtual-key on Windows, CGKeyCode on macOS) like 0x25

h=Left
j=Down
k=Up
l=Right
```

## Notes
- CapsLock is swallowed and used purely as a momentary modifier; tapping it does not toggle caps state.
- If you want CapsLock to remain off, ensure it’s off before launching (or modify the code to force it off at startup).
- On macOS the app stays attached to your terminal session; press `Ctrl+C` to exit.

## License
No license specified by default; add one if needed for distribution.
