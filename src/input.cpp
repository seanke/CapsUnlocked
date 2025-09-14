#include "input.h"

static const ULONG_PTR INJECT_MARKER = 0xBEEFCAFE;

bool is_extended_key(UINT vk) {
    switch (vk) {
        case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
        case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END:
        case VK_PRIOR: case VK_NEXT: case VK_DIVIDE: case VK_NUMLOCK:
        case VK_RCONTROL: case VK_RMENU: case VK_SNAPSHOT: case VK_CANCEL:
        case VK_LWIN: case VK_RWIN: case VK_APPS:
            return true;
        default:
            return false;
    }
}

void send_key(UINT vk, bool down) {
    INPUT in{};
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = static_cast<WORD>(vk);
    in.ki.dwExtraInfo = INJECT_MARKER;
    if (!down) in.ki.dwFlags |= KEYEVENTF_KEYUP;
    if (is_extended_key(vk)) in.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
    SendInput(1, &in, sizeof(in));
}

bool is_our_injected(const KBDLLHOOKSTRUCT& kb) {
    return kb.dwExtraInfo == INJECT_MARKER;
}

