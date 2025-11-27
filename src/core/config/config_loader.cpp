#include "config_loader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace caps::core {

namespace {

// Returns true if the current line begins with comment prefixes after trimming.
bool IsComment(const std::string& line) {
    for (char ch : line) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        return ch == '#' || ch == ';';
    }
    return false;
}

} // namespace

ConfigLoader::ConfigLoader()
    : mappings_(BuildDefaultMappings()) {}

// Reads the config at `path`, remembering it so Reload() can reuse the same source.
void ConfigLoader::Load(const std::string& path) {
    config_path_ = path;
    mappings_ = ParseConfigFile(path);
}

// Convenience helper for hot-reloads; uses the last path passed into Load().
void ConfigLoader::Reload() {
    if (config_path_.empty()) {
        throw std::runtime_error("ConfigLoader::Reload called before Load");
    }

    mappings_ = ParseConfigFile(config_path_);
}

const ConfigLoader::MappingTable& ConfigLoader::Mappings() const {
    return mappings_;
}

// Produces a quick human-readable summary that is handy for logging and debugging.
std::string ConfigLoader::Describe() const {
    std::ostringstream output;
    size_t count = 0;
    for (const auto& [app, mod_table] : mappings_) {
        for (const auto& [modifier, key_table] : mod_table) {
            count += key_table.size();
        }
    }
    output << "Config (" << count << " entries)";
    for (const auto& [app, mod_table] : mappings_) {
        for (const auto& [modifier, key_table] : mod_table) {
            for (const auto& [source, target] : key_table) {
                output << "\n[" << app << "][" << modifier << "] " << source << " -> " << target;
            }
        }
    }
    return output.str();
}

// Opens the ini file, parses whitespace-delimited rows, and returns a normalized map.
// Format: app modifier source target
// Where modifier can be * (no additional modifier), A, S, or A+S
ConfigLoader::MappingTable ConfigLoader::ParseConfigFile(const std::string& path) const {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        return BuildDefaultMappings();
    }

    MappingTable parsed;
    std::string line;
    size_t line_number = 0;
    while (std::getline(stream, line)) {
        ++line_number;
        const std::string trimmed = Trim(line);
        if (trimmed.empty() || IsComment(trimmed)) {
            continue;
        }

        if (trimmed.find('=') != std::string::npos) {
            throw std::runtime_error("Invalid config line " + std::to_string(line_number) +
                                     ": '=' separators are not supported; use whitespace");
        }

        std::istringstream line_stream(trimmed);
        std::string app_token;
        std::string modifier_token;
        std::string key_token;
        std::string action_token;
        if (!(line_stream >> app_token >> modifier_token >> key_token >> action_token)) {
            throw std::runtime_error("Invalid config line " + std::to_string(line_number) +
                                     ": expected 'app modifier key action'");
        }

        // Normalize tokens so lookups are case-insensitive and whitespace-agnostic.
        const std::string app = NormalizeAppToken(app_token);
        const std::string modifier = NormalizeModifierToken(modifier_token);
        const std::string source = NormalizeKeyToken(key_token);
        const std::string target = NormalizeKeyToken(action_token);

        parsed[app][modifier][source] = target;
    }

    if (parsed.empty()) {
        return BuildDefaultMappings();
    }

    return parsed;
}

// Default vim-style arrows that keep the product useful when no config exists.
// Format: app -> modifier -> source -> target
ConfigLoader::MappingTable ConfigLoader::BuildDefaultMappings() {
    return {
        {"*", {
            {"*", {{"H", "LEFT"}, {"J", "DOWN"}, {"K", "UP"}, {"L", "RIGHT"}}},
            {"A", {{"J", "END"}, {"K", "HOME"}, {"I", "PAGEUP"}, {"L", "PAGEDOWN"}}},
            {"S", {{"H", "SHIFT+LEFT"}, {"J", "SHIFT+DOWN"}, {"K", "SHIFT+UP"}, {"L", "SHIFT+RIGHT"}}},
            {"A+S", {{"J", "SHIFT+END"}, {"K", "SHIFT+HOME"}, {"I", "SHIFT+PAGEUP"}, {"L", "SHIFT+PAGEDOWN"}}}
        }},
    };
}

// Uppercases and strips whitespace so that config lookups become case-insensitive.
std::string ConfigLoader::NormalizeKeyToken(const std::string& token) {
    const std::string trimmed = Trim(token);
    if (trimmed.empty()) {
        throw std::runtime_error("Empty key token in config file");
    }

    std::string normalized;
    normalized.reserve(trimmed.size());

    bool previous_was_space = false;
    for (char ch : trimmed) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!previous_was_space) {
                normalized.push_back(' ');
                previous_was_space = true;
            }
            continue;
        }

        previous_was_space = false;
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }

    if (!normalized.empty() && normalized.back() == ' ') {
        normalized.pop_back();
    }

    if (normalized.empty()) {
        throw std::runtime_error("Key token reduced to empty value during normalization");
    }

    return normalized;
}

std::string ConfigLoader::NormalizeAppToken(const std::string& token) {
    const std::string trimmed = Trim(token);
    if (trimmed.empty()) {
        return "*";
    }

    std::string normalized;
    normalized.reserve(trimmed.size());
    for (char ch : trimmed) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            continue;
        }
        normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
    }
    if (normalized.empty()) {
        return "*";
    }
    return normalized;
}

// Normalizes modifier tokens like "*", "A", "S", "A+S"
// Sorts multi-key modifiers alphabetically so "S+A" becomes "A+S"
std::string ConfigLoader::NormalizeModifierToken(const std::string& token) {
    const std::string trimmed = Trim(token);
    if (trimmed.empty() || trimmed == "*") {
        return "*";
    }

    // Split by '+' and collect individual modifier keys
    std::vector<std::string> parts;
    std::istringstream stream(trimmed);
    std::string part;
    while (std::getline(stream, part, '+')) {
        std::string normalized_part;
        for (char ch : part) {
            if (!std::isspace(static_cast<unsigned char>(ch))) {
                normalized_part.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
            }
        }
        if (!normalized_part.empty() && normalized_part != "*") {
            parts.push_back(normalized_part);
        }
    }

    if (parts.empty()) {
        return "*";
    }

    // Sort alphabetically for consistent lookups
    std::sort(parts.begin(), parts.end());

    // Remove duplicates
    parts.erase(std::unique(parts.begin(), parts.end()), parts.end());

    // Join with '+'
    std::string result = parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        result += '+';
        result += parts[i];
    }
    return result;
}

// Minimal std::string trim helper that avoids pulling in boost/Qt/etc.
std::string ConfigLoader::Trim(const std::string& value) {
    const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    });
    if (begin == value.end()) {
        return "";
    }
    const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
        return std::isspace(ch) != 0;
    }).base();
    return std::string(begin, end);
}

} // namespace caps::core
