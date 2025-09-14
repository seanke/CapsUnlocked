# CapsUnlocked (Windows 11)

A small Windows keyboard layer tool that turns `CapsLock` into a momentary modifier. While held, selected keys are remapped based on a config file. By default, `h j k l` become the arrow keys. Runs in the system tray with a right-click Exit menu.

## Features
- Hold `CapsLock` to activate the layer
- Remap keys via `capsunlocked.ini` next to the executable
- Default mapping: `h=Left`, `j=Down`, `k=Up`, `l=Right`
- Uses a low-level keyboard hook and `SendInput`

## Build
Prereqs: CMake 3.16+, Visual Studio (MSVC), Windows 11

```powershell
# from the repo root
cmake -S . -B build -A x64
cmake --build build --config Release
```
The resulting binary is at `build/Release/CapsUnlocked.exe` (or under your chosen config).

## Run
Just run the EXE. A tray icon appears (default app icon). Right-click the tray icon and choose `Exit` to close.

On first run it writes a default `capsunlocked.ini` next to the EXE if it can. If not, it falls back to the default mapping internally.

Note: To intercept keystrokes for elevated apps (run as Administrator), run CapsUnlocked elevated as well due to Windows UIPI.

## Config
Config file path: `./capsunlocked.ini` (same folder as the EXE)

Format:
```
# Lines like source=target
# Supported tokens: single characters (e.g. h, a) and common names (Left, Right, Up, Down, Enter, Esc, Tab, Space, Backspace, Delete, Home, End, PageUp, PageDown), or hex VK like 0x25

h=Left
j=Down
k=Up
l=Right
```

## Notes
- CapsLock is swallowed and used purely as a momentary modifier; tapping it does not toggle caps state.
- If you want CapsLock to remain off, ensure it’s off before launching (or modify the code to force it off at startup).
- Close the console window or press `Ctrl+C` to exit.

## License
No license specified by default; add one if needed for distribution.
