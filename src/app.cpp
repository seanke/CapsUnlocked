#include <windows.h>
#include <shellapi.h>
#include <string>

#include "hook.h"
#include "config.h"

static HWND g_hwnd = nullptr;
static const wchar_t* APP_CLASS_NAME = L"CapsUnlockedHiddenWindow";
static const UINT WM_APP_TRAY = WM_APP + 1;
static const UINT TRAY_UID = 1001;
static const UINT IDM_EXIT = 40001;

static void tray_add(HWND hwnd) {
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = TRAY_UID;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_APP_TRAY;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcsncpy_s(nid.szTip, L"CapsUnlocked: Hold CapsLock for layer", _TRUNCATE);
    Shell_NotifyIconW(NIM_ADD, &nid);
}

static void tray_delete(HWND hwnd) {
    NOTIFYICONDATAW nid{};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uID = TRAY_UID;
    Shell_NotifyIconW(NIM_DELETE, &nid);
}

static void tray_show_menu(HWND hwnd) {
    HMENU menu = CreatePopupMenu();
    if (!menu) return;
    AppendMenuW(menu, MF_STRING, IDM_EXIT, L"Exit");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    TrackPopupMenu(menu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, nullptr);
    DestroyMenu(menu);
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            tray_add(hwnd);
            return 0;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_EXIT:
                    DestroyWindow(hwnd);
                    return 0;
                default:
                    break;
            }
            break;
        case WM_APP_TRAY:
            if (lParam == WM_RBUTTONUP || lParam == WM_CONTEXTMENU) {
                tray_show_menu(hwnd);
                return 0;
            }
            break;
        case WM_DESTROY:
            tray_delete(hwnd);
            uninstall_hook();
            PostQuitMessage(0);
            return 0;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    // Load mapping from config
    auto mapping = load_config();
    set_mapping(mapping);

    if (!install_hook()) {
        MessageBoxW(nullptr, L"Failed to install keyboard hook.", L"CapsUnlocked", MB_ICONERROR | MB_OK);
        return 1;
    }

    WNDCLASSW wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = APP_CLASS_NAME;
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    if (!RegisterClassW(&wc)) {
        uninstall_hook();
        MessageBoxW(nullptr, L"Failed to register window class.", L"CapsUnlocked", MB_ICONERROR | MB_OK);
        return 1;
    }

    g_hwnd = CreateWindowExW(0, APP_CLASS_NAME, APP_CLASS_NAME, WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                             nullptr, nullptr, hInstance, nullptr);
    if (!g_hwnd) {
        uninstall_hook();
        MessageBoxW(nullptr, L"Failed to create hidden window.", L"CapsUnlocked", MB_ICONERROR | MB_OK);
        return 1;
    }

    ShowWindow(g_hwnd, SW_HIDE);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

