#pragma once

#include "platform.h"
#include <unordered_map>
#include <string>

using KeyMapping = std::unordered_map<KeyCode, KeyCode>;

// Loads config from capsunlocked.ini next to the executable.
// If file missing or empty, returns default hjkl->arrows map.
KeyMapping load_config();
