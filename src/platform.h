#pragma once

#if defined(_WIN32)
#include <windows.h>
using KeyCode = UINT;
constexpr KeyCode CAPS_LOCK_KEY = VK_CAPITAL;
#else
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
using KeyCode = CGKeyCode;
constexpr KeyCode CAPS_LOCK_KEY = kVK_CapsLock;
#endif
