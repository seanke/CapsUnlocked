#pragma once

#include <windows.h>
#include <unordered_map>
#include <string>

using KeyMap = std::unordered_map<UINT, UINT>;

// Loads config from capsunlocked.ini next to the executable.
// If file missing or empty, returns default hjkl->arrows map.
KeyMap load_config();

