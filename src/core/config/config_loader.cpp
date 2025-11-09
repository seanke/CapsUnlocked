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
    output << "Config (" << mappings_.size() << " entries)";
    for (const auto& [source, target] : mappings_) {
        output << "\n" << source << " -> " << target;
    }
    return output.str();
}

// Opens the ini file, parses `key=value` rows, and returns a normalized map.
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

        const auto separator = trimmed.find('=');
        if (separator == std::string::npos) {
            throw std::runtime_error("Invalid config line " + std::to_string(line_number) + ": missing '='");
        }

        const std::string left = trimmed.substr(0, separator);
        const std::string right = trimmed.substr(separator + 1);

        // Normalize both halves so the lookup table stays case-insensitive.
        const std::string source = NormalizeKeyToken(left);
        const std::string target = NormalizeKeyToken(right);

        parsed[source] = target;
    }

    if (parsed.empty()) {
        return BuildDefaultMappings();
    }

    return parsed;
}

// Default vim-style arrows that keep the product useful when no config exists.
ConfigLoader::MappingTable ConfigLoader::BuildDefaultMappings() {
    return {
        {"H", "LEFT"},
        {"J", "DOWN"},
        {"K", "UP"},
        {"L", "RIGHT"},
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
