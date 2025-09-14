#pragma once

#include <windows.h>
#include <string>

std::wstring get_exe_dir();
void trim(std::wstring& s);
UINT token_to_vk(const std::wstring& token);

