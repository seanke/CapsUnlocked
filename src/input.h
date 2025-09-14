#pragma once

#include <windows.h>

bool is_extended_key(UINT vk);
void send_key(UINT vk, bool down);
bool is_our_injected(const KBDLLHOOKSTRUCT& kb);

