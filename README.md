# CapsUnlocked (Windows 11 & macOS)

A small keyboard layer tool that turns `CapsLock` into a momentary modifier. While held, selected keys are remapped based on a config file. By default, `j k i l` become the arrow keys (`j` left, `l` right, `i` up, `k` down) and holding `a` adds navigation combos (`a+j` = Home, `a+k` = PageDown, `a+i` = PageUp, `a+l` = End). On Windows it lives in the system tray with a right-click Exit menu; on macOS it runs as a background console app.

## Features
- Hold `CapsLock` to activate the layer
- Remap keys via `capsunlocked.ini` next to the executable
- Default mapping: `j=Left`, `k=Down`, `i=Up`, `l=Right`
- Default modifier combos: hold `a` for `j=Home`, `k=PageDown`, `i=PageUp`, `l=End`
- **Layer Modifiers**: Define custom modifier keys that, when held along with CapsLock, activate alternative mappings
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

### Basic Config (matches the default)
Defaults with a single modifier (`a`) for navigation:
```ini
[modifiers]
a

[maps]
*                   j   Left
*                   k   Down
*                   i   Up
*                   l   Right

[*] [a] [j] [Home]
[*] [a] [k] [PageDown]
[*] [a] [i] [PageUp]
[*] [a] [l] [End]
```

### Extended Format with Modifiers
For advanced layer control, you can define modifier keys and create conditional mappings:

```ini
[modifiers]
a
s

[maps]
# When both 'a' and 's' are held with CapsLock, pressing 'j' sends Shift+End
[*] [a s] [j] [Shift End]

# When neither 'a' nor 's' are held, pressing 'j' sends Down
[*] [] [j] [Down]

# Regular mapping without modifiers
[*] [h] [Left]
```

### Modifiers Section
- **`[modifiers]`**: Optional section listing keys that act as layer modifiers
- Each line contains a single key name (e.g., `a`, `s`, `Shift`, `Ctrl`)
- Modifier keys are reserved and cannot be used as source or target keys in mappings
- When you press a modifier key while the layer is active, it's swallowed (not sent through)
- Modifiers use logical AND: all listed modifiers must be held for a mapping to activate

### Mapping Syntax
- **Legacy format**: `app source target` (whitespace-delimited)
- **Bracket format**: `[app] [source] [target]` (no modifiers)
- **With modifiers**: `[app] [mods] [source] [target]`
  - `mods` is a space-separated list of modifiers that must be held
  - Use `[]` for an empty modifier list (fallback mapping)

### Mapping Priority
When multiple mappings exist for the same source key, the most specific one (with the most matching modifiers) takes priority.

### Supported Tokens
- Single characters: `h`, `a`, `j`
- Common key names: `Left`, `Right`, `Up`, `Down`, `Enter`, `Esc`, `Tab`, `Space`, `Backspace`, `Delete`, `Home`, `End`, `PageUp`, `PageDown`, `Shift`, `Ctrl`, `Alt`
- Hex key codes: `0x25` (virtual-key on Windows, CGKeyCode on macOS)

### Validation Rules
When a `[modifiers]` section is present:
1. Keys declared as modifiers cannot be used as source keys in mappings
2. Keys declared as modifiers cannot be used as target keys in mappings
3. Modifiers used in mapping modifier lists must be defined in the `[modifiers]` section

If no `[modifiers]` section exists, the config file behaves as before (backwards compatible).

## Notes
- CapsLock is swallowed and used purely as a momentary modifier; tapping it does not toggle caps state.
- If you want CapsLock to remain off, ensure it's off before launching (or modify the code to force it off at startup).
- On macOS the app stays attached to your terminal session; press `Ctrl+C` to exit.

## License
No license specified by default; add one if needed for distribution.
