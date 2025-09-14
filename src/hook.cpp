#include "hook.h"
#include "input.h"
#include <cwctype>

static HHOOK g_hook = nullptr;
static bool g_capsLayerActive = false;
static std::unordered_map<UINT, UINT> g_layerMap;

void set_mapping(const std::unordered_map<UINT, UINT>& map) {
    g_layerMap = map;
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode < 0) {
        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    const KBDLLHOOKSTRUCT* pkb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    const bool isKeyUp = (pkb->flags & LLKHF_UP) != 0;
    const UINT vk = pkb->vkCode;

    if (is_our_injected(*pkb)) {
        return CallNextHookEx(g_hook, nCode, wParam, lParam);
    }

    if (vk == VK_CAPITAL) {
        if (!isKeyUp) g_capsLayerActive = true; else g_capsLayerActive = false;
        return 1; // block default CapsLock behavior
    }

    if (g_capsLayerActive) {
        auto it = g_layerMap.find(vk);
        if (it == g_layerMap.end()) {
            UINT upper = vk;
            if (vk >= 'a' && vk <= 'z') upper = static_cast<UINT>(std::towupper(static_cast<wchar_t>(vk)));
            it = g_layerMap.find(upper);
        }
        if (it != g_layerMap.end()) {
            send_key(it->second, !isKeyUp);
            return 1;
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

bool install_hook() {
    if (g_hook) return true;
    g_hook = SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandleW(nullptr), 0);
    return g_hook != nullptr;
}

void uninstall_hook() {
    if (g_hook) {
        UnhookWindowsHookEx(g_hook);
        g_hook = nullptr;
    }
}

