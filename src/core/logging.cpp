#include "core/logging.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace caps::core::logging {

namespace {

std::atomic<Level> g_level{Level::Debug};
std::mutex g_stream_mutex;

const char* ToString(Level level) {
    switch (level) {
        case Level::Debug:
            return "DEBUG";
        case Level::Info:
            return "INFO";
        case Level::Warning:
            return "WARN";
        case Level::Error:
            return "ERROR";
    }
    return "INFO";
}

bool ShouldLog(Level level) {
    return static_cast<int>(level) >= static_cast<int>(g_level.load(std::memory_order_relaxed));
}

std::ostream& StreamFor(Level level) {
    if (level == Level::Warning || level == Level::Error) {
        return std::cerr;
    }
    return std::cout;
}

std::string Timestamp() {
    using clock = std::chrono::system_clock;
    const auto now = clock::now();
    const auto time = clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::ostringstream os;
    os << std::put_time(&tm, "%H:%M:%S");
    return os.str();
}

} // namespace

Level GetLevel() {
    return g_level.load(std::memory_order_relaxed);
}

void SetLevel(Level level) {
    g_level.store(level, std::memory_order_relaxed);
}

void Log(Level level, std::string_view message) {
    if (!ShouldLog(level)) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_stream_mutex);
    auto& out = StreamFor(level);
    out << "[" << Timestamp() << "] [" << ToString(level) << "] " << message << std::endl;
}

} // namespace caps::core::logging
