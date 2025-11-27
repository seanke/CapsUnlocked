#pragma once

#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace caps::core {

// Shared string utilities for key/modifier/token normalization.
// Used across config loading, mapping resolution, and layer control.

// Normalizes a key token by removing whitespace and converting to uppercase.
inline std::string NormalizeKeyToken(const std::string& key) {
    std::string normalized;
    normalized.reserve(key.size());
    for (char ch : key) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        }
    }
    return normalized;
}

// Normalizes an app token by removing whitespace and converting to uppercase.
// Returns "*" for empty or whitespace-only input.
inline std::string NormalizeAppToken(const std::string& app) {
    if (app.empty()) {
        return "*";
    }

    std::string normalized;
    normalized.reserve(app.size());
    for (char ch : app) {
        if (!std::isspace(static_cast<unsigned char>(ch))) {
            normalized.push_back(static_cast<char>(std::toupper(static_cast<unsigned char>(ch))));
        }
    }
    if (normalized.empty()) {
        return "*";
    }
    return normalized;
}

// Converts a set of modifier keys to a normalized string like "A+S".
// Returns "*" for empty sets.
inline std::string NormalizeModifierSet(const std::set<std::string>& modifiers) {
    if (modifiers.empty()) {
        return "*";
    }

    std::vector<std::string> sorted_mods;
    for (const auto& mod : modifiers) {
        std::string normalized = NormalizeKeyToken(mod);
        if (!normalized.empty() && normalized != "*") {
            sorted_mods.push_back(normalized);
        }
    }

    if (sorted_mods.empty()) {
        return "*";
    }

    std::sort(sorted_mods.begin(), sorted_mods.end());
    sorted_mods.erase(std::unique(sorted_mods.begin(), sorted_mods.end()), sorted_mods.end());

    std::string result = sorted_mods[0];
    for (size_t i = 1; i < sorted_mods.size(); ++i) {
        result += '+';
        result += sorted_mods[i];
    }
    return result;
}

// Normalizes a modifier token string like "A+S", "S+A", or "*".
// Sorts and deduplicates multi-key modifiers so "S+A" becomes "A+S".
inline std::string NormalizeModifierToken(const std::string& token) {
    std::string trimmed = NormalizeKeyToken(token);
    if (trimmed.empty() || trimmed == "*") {
        return "*";
    }

    // If no '+' present, it's a single modifier
    if (trimmed.find('+') == std::string::npos) {
        return trimmed;
    }

    // Split by '+' and collect individual modifier keys
    std::set<std::string> parts;
    std::istringstream stream(trimmed);
    std::string part;
    while (std::getline(stream, part, '+')) {
        std::string normalized = NormalizeKeyToken(part);
        if (!normalized.empty() && normalized != "*") {
            parts.insert(normalized);
        }
    }

    return NormalizeModifierSet(parts);
}

} // namespace caps::core
