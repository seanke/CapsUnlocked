#pragma once

#include "platform.h"

void send_key(KeyCode vk, bool down);

#if defined(_WIN32)
bool is_extended_key(KeyCode vk);
bool is_our_injected(const KBDLLHOOKSTRUCT& kb);
#else
bool is_our_injected(CGEventRef event);
#endif

