#pragma once

#include <windows.h>
#include <unordered_map>

void set_mapping(const std::unordered_map<UINT, UINT>& map);
bool install_hook();
void uninstall_hook();

