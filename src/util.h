#pragma once

#include "platform.h"
#include <filesystem>
#include <string>

std::filesystem::path get_exe_dir();
void trim(std::string& s);
KeyCode token_to_keycode(const std::string& token);
