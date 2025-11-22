#include "app_monitor.h"

#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>
#include <libproc.h>
#include <unistd.h>

#include <optional>
#include <string>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

namespace caps::platform::macos {

namespace {

std::optional<ProcessSerialNumber> FrontmostProcess() {
    ProcessSerialNumber psn;
    if (GetFrontProcess(&psn) != noErr) {
        return std::nullopt;
    }
    return psn;
}

pid_t PidFromProcessSerial(const ProcessSerialNumber& psn) {
    pid_t pid = -1;
    GetProcessPID(&psn, &pid);
    return pid;
}

std::string CFStringToStd(CFStringRef str) {
    if (!str) {
        return "";
    }
    char buffer[PATH_MAX] = {};
    if (CFStringGetCString(str, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
        return std::string(buffer);
    }
    return "";
}

} // namespace

std::string AppMonitor::CurrentAppName() const {
    auto psn = FrontmostProcess();
    pid_t pid = -1;
    if (psn) {
        pid = PidFromProcessSerial(*psn);
    }

    if (psn) {
        CFDictionaryRef info =
            ProcessInformationCopyDictionary(&*psn, kProcessDictionaryIncludeAllInformationMask);
        if (info) {
            CFStringRef bundle_id =
                static_cast<CFStringRef>(CFDictionaryGetValue(info, CFSTR("BundleIdentifier")));
            std::string bundle = CFStringToStd(bundle_id);
            CFRelease(info);
            if (!bundle.empty()) {
                return bundle;
            }
        }
    }

    if (pid <= 0) {
        return "";
    }

    char path_buffer[PROC_PIDPATHINFO_MAXSIZE] = {};
    if (proc_pidpath(pid, path_buffer, sizeof(path_buffer)) > 0) {
        std::string full_path(path_buffer);
        const auto app_pos = full_path.find(".app");
        if (app_pos != std::string::npos) {
            const auto slash = full_path.rfind('/', app_pos);
            size_t start = (slash == std::string::npos) ? 0 : slash + 1;
            if (app_pos > start) {
                std::string app_dir = full_path.substr(start, app_pos - start);
                if (!app_dir.empty()) {
                    return app_dir;
                }
            }
        }
        const auto slash = full_path.find_last_of('/');
        if (slash != std::string::npos && slash + 1 < full_path.size()) {
            return full_path.substr(slash + 1);
        }
        return full_path;
    }

    char name[PROC_PIDPATHINFO_MAXSIZE] = {};
    const int length = proc_name(pid, name, sizeof(name));
    if (length > 0) {
        return std::string(name, static_cast<size_t>(length));
    }

    return "";
}

} // namespace caps::platform::macos

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
