#pragma once

#include <atomic>
#include <string_view>
#include <optional>

namespace caps::core::logging {

enum class Level {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
};

// Returns the current global log level threshold. Messages below this level are suppressed.
Level GetLevel();
// Sets the global log level threshold.
void SetLevel(Level level);
// Parses a string name into a Level (case-insensitive). Returns std::nullopt on failure.
std::optional<Level> ParseLevel(std::string_view name);

// Emits a log message at the given level if it meets the current threshold.
void Log(Level level, std::string_view message);

// Convenience helpers for common log levels.
inline void Debug(std::string_view message) { Log(Level::Debug, message); }
inline void Info(std::string_view message) { Log(Level::Info, message); }
inline void Warn(std::string_view message) { Log(Level::Warning, message); }
inline void Error(std::string_view message) { Log(Level::Error, message); }

} // namespace caps::core::logging
