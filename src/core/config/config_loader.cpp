#include "config_loader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

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
    for (const auto& [app, table] : mappings_) {
        count += table.size();
    }
    output << "Config (" << count << " entries)";
    for (const auto& [app, table] : mappings_) {
        for (const auto& [source, target] : table) {
            output << "\n[" << app << "] " << source << " -> " << target;
        }
    }
    return output.str();
}

// Opens the ini file, parses whitespace-delimited `key value` rows, and returns a normalized map.
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
        std::string key_token;
        std::string action_token;
        if (!(line_stream >> app_token >> key_token >> action_token)) {
            throw std::runtime_error("Invalid config line " + std::to_string(line_number) +
                                     ": expected 'app key action'");
        }

        // Normalize tokens so lookups are case-insensitive and whitespace-agnostic.
        const std::string app = NormalizeAppToken(app_token);
        const std::string source = NormalizeKeyToken(key_token);
        const std::string target = NormalizeKeyToken(action_token);

        parsed[app][source] = target;
    }

    if (parsed.empty()) {
        return BuildDefaultMappings();
    }

    return parsed;
}

// Default vim-style arrows that keep the product useful when no config exists.
ConfigLoader::MappingTable ConfigLoader::BuildDefaultMappings() {
    return {
        {"*", {{"H", "LEFT"}, {"J", "DOWN"}, {"K", "UP"}, {"L", "RIGHT"}}},
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
