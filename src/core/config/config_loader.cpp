#include "config_loader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <regex>
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

// Section type enumeration for INI parsing
enum class SectionType { None, Maps, Modifiers };

// Parse a section header like [modifiers] or [maps]
// Returns the section type if recognized, or None if unrecognized
SectionType ParseSectionHeader(const std::string& line) {
    std::string trimmed = ConfigLoader::Trim(line);
    if (trimmed.empty() || trimmed.front() != '[' || trimmed.back() != ']') {
        return SectionType::None;
    }
    
    std::string section_name = trimmed.substr(1, trimmed.size() - 2);
    // Normalize to lowercase for comparison
    std::transform(section_name.begin(), section_name.end(), section_name.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    
    if (section_name == "modifiers") {
        return SectionType::Modifiers;
    }
    if (section_name == "maps") {
        return SectionType::Maps;
    }
    return SectionType::None;
}

// Check if a line is a section header (single bracket group only, e.g. [modifiers] or [maps])
bool IsSectionHeader(const std::string& line) {
    std::string trimmed = ConfigLoader::Trim(line);
    if (trimmed.empty() || trimmed.front() != '[') {
        return false;
    }
    // Find the first closing bracket
    size_t close = trimmed.find(']');
    if (close == std::string::npos) {
        return false;
    }
    // Section header: nothing after the first closing bracket (except whitespace)
    std::string after = ConfigLoader::Trim(trimmed.substr(close + 1));
    return after.empty();
}

// Parse bracket-delimited tokens from a mapping line
// Format: [app] [mods] [source] [target] OR [app] [source] [target]
// Where brackets can contain space-separated keys
struct ParsedMapping {
    std::string app;
    std::vector<std::string> modifiers;
    std::string source;
    std::string target;
    bool valid{false};
    std::string error;
};

ParsedMapping ParseMappingLine(const std::string& line, size_t line_number) {
    ParsedMapping result;
    
    // Match bracket groups: [content] - static regex to avoid recompilation overhead
    static const std::regex bracket_regex(R"(\[([^\]]*)\])");
    std::vector<std::string> groups;
    
    auto begin = std::sregex_iterator(line.begin(), line.end(), bracket_regex);
    auto end = std::sregex_iterator();
    
    for (auto it = begin; it != end; ++it) {
        groups.push_back((*it)[1].str());
    }
    
    if (groups.size() == 3) {
        // Format: [app] [source] [target] - no modifiers
        result.app = ConfigLoader::NormalizeAppToken(groups[0]);
        result.source = ConfigLoader::NormalizeKeyToken(ConfigLoader::Trim(groups[1]));
        result.target = ConfigLoader::NormalizeKeyToken(ConfigLoader::Trim(groups[2]));
        result.valid = true;
    } else if (groups.size() == 4) {
        // Format: [app] [mods] [source] [target]
        result.app = ConfigLoader::NormalizeAppToken(groups[0]);
        
        // Parse modifiers (space-separated keys in the second bracket)
        std::string mods_str = ConfigLoader::Trim(groups[1]);
        if (!mods_str.empty()) {
            std::istringstream mods_stream(mods_str);
            std::string mod;
            while (mods_stream >> mod) {
                result.modifiers.push_back(ConfigLoader::NormalizeKeyToken(mod));
            }
        }
        
        result.source = ConfigLoader::NormalizeKeyToken(ConfigLoader::Trim(groups[2]));
        result.target = ConfigLoader::NormalizeKeyToken(ConfigLoader::Trim(groups[3]));
        result.valid = true;
    } else if (groups.empty()) {
        // Fall back to legacy whitespace-delimited format: app source target
        std::string trimmed = ConfigLoader::Trim(line);
        if (trimmed.find('=') != std::string::npos) {
            result.error = "Invalid config line " + std::to_string(line_number) +
                          ": '=' separators are not supported; use whitespace or brackets";
            return result;
        }
        
        std::istringstream line_stream(trimmed);
        std::string app_token, key_token, action_token;
        if (!(line_stream >> app_token >> key_token >> action_token)) {
            result.error = "Invalid config line " + std::to_string(line_number) +
                          ": expected '[app] [source] [target]' or 'app source target'";
            return result;
        }
        
        result.app = ConfigLoader::NormalizeAppToken(app_token);
        result.source = ConfigLoader::NormalizeKeyToken(key_token);
        result.target = ConfigLoader::NormalizeKeyToken(action_token);
        result.valid = true;
    } else {
        result.error = "Invalid config line " + std::to_string(line_number) +
                      ": expected 3 or 4 bracket groups, found " + std::to_string(groups.size());
    }
    
    return result;
}

} // namespace

ConfigLoader::ConfigLoader()
    : mappings_(BuildDefaultMappings()),
      modifiers_(BuildDefaultModifiers()),
      has_modifiers_section_(true) {}

// Reads the config at `path`, remembering it so Reload() can reuse the same source.
void ConfigLoader::Load(const std::string& path) {
    config_path_ = path;
    auto result = ParseConfigFile(path);
    mappings_ = std::move(result.mappings);
    modifiers_ = std::move(result.modifiers);
    has_modifiers_section_ = result.has_modifiers_section;
}

// Convenience helper for hot-reloads; uses the last path passed into Load().
void ConfigLoader::Reload() {
    if (config_path_.empty()) {
        throw std::runtime_error("ConfigLoader::Reload called before Load");
    }

    auto result = ParseConfigFile(config_path_);
    mappings_ = std::move(result.mappings);
    modifiers_ = std::move(result.modifiers);
    has_modifiers_section_ = result.has_modifiers_section;
}

const ConfigLoader::MappingTable& ConfigLoader::Mappings() const {
    return mappings_;
}

const ConfigLoader::ModifierSet& ConfigLoader::Modifiers() const {
    return modifiers_;
}

bool ConfigLoader::HasModifiersSection() const {
    return has_modifiers_section_;
}

// Produces a quick human-readable summary that is handy for logging and debugging.
std::string ConfigLoader::Describe() const {
    std::ostringstream output;
    size_t count = 0;
    for (const auto& [app, definitions] : mappings_) {
        count += definitions.size();
    }
    output << "Config (" << count << " entries";
    if (has_modifiers_section_) {
        output << ", " << modifiers_.size() << " modifiers";
    }
    output << ")";
    
    if (has_modifiers_section_ && !modifiers_.empty()) {
        output << "\nModifiers: ";
        bool first = true;
        for (const auto& mod : modifiers_) {
            if (!first) output << ", ";
            output << mod;
            first = false;
        }
    }
    
    for (const auto& [app, definitions] : mappings_) {
        for (const auto& def : definitions) {
            output << "\n[" << app << "] ";
            if (!def.required_mods.empty()) {
                output << "[";
                for (size_t i = 0; i < def.required_mods.size(); ++i) {
                    if (i > 0) output << " ";
                    output << def.required_mods[i];
                }
                output << "] ";
            }
            output << def.source << " -> " << def.target;
        }
    }
    return output.str();
}

// Opens the ini file, parses sections and mapping lines.
ConfigLoader::ParseResult ConfigLoader::ParseConfigFile(const std::string& path) const {
    std::ifstream stream(path);
    if (!stream.is_open()) {
        ParseResult result;
        result.mappings = BuildDefaultMappings();
        result.modifiers = BuildDefaultModifiers();
        result.has_modifiers_section = true;
        return result;
    }

    ParseResult result;
    std::string line;
    size_t line_number = 0;
    SectionType current_section = SectionType::None;
    
    while (std::getline(stream, line)) {
        ++line_number;
        const std::string trimmed = Trim(line);
        if (trimmed.empty() || IsComment(trimmed)) {
            continue;
        }

        // Check for section header
        if (IsSectionHeader(trimmed)) {
            SectionType new_section = ParseSectionHeader(trimmed);
            if (new_section != SectionType::None) {
                current_section = new_section;
                if (current_section == SectionType::Modifiers) {
                    result.has_modifiers_section = true;
                }
            }
            // Ignore unrecognized sections
            continue;
        }

        // Process line based on current section
        if (current_section == SectionType::Modifiers) {
            // Each line in [modifiers] is a single key name
            std::string mod_key = NormalizeKeyToken(trimmed);
            result.modifiers.insert(mod_key);
        } else {
            // Default section or [maps] section: parse mapping lines
            auto parsed = ParseMappingLine(trimmed, line_number);
            if (!parsed.valid) {
                throw std::runtime_error(parsed.error);
            }
            
            // Validation: if modifiers section exists, check constraints
            if (result.has_modifiers_section) {
                // Check that source key is not a modifier (unless it's in the modifier list)
                if (result.modifiers.count(parsed.source) > 0) {
                    throw std::runtime_error("Invalid config line " + std::to_string(line_number) +
                                           ": source key '" + parsed.source + 
                                           "' is declared as a modifier and cannot be used as a source key");
                }
                
                // Check that target key is not a modifier
                // Note: target could be space-separated for multi-key output
                std::istringstream target_stream(parsed.target);
                std::string target_key;
                while (target_stream >> target_key) {
                    if (result.modifiers.count(target_key) > 0) {
                        throw std::runtime_error("Invalid config line " + std::to_string(line_number) +
                                               ": target key '" + target_key + 
                                               "' is declared as a modifier and cannot be used as a target key");
                    }
                }
                
                // Check that all modifiers in the mapping are defined
                for (const auto& mod : parsed.modifiers) {
                    if (result.modifiers.count(mod) == 0) {
                        throw std::runtime_error("Invalid config line " + std::to_string(line_number) +
                                               ": modifier '" + mod + 
                                               "' used in mapping but not declared in [modifiers] section");
                    }
                }
            }
            
            MappingDefinition def;
            def.source = parsed.source;
            def.target = parsed.target;
            def.required_mods = std::move(parsed.modifiers);
            
            result.mappings[parsed.app].push_back(std::move(def));
        }
    }

    if (result.mappings.empty()) {
        result.mappings = BuildDefaultMappings();
        if (result.modifiers.empty()) {
            result.modifiers = BuildDefaultModifiers();
        }
        result.has_modifiers_section = true;
    }

    return result;
}

// Default arrow keys that keep the product useful when no config exists.
ConfigLoader::MappingTable ConfigLoader::BuildDefaultMappings() {
    return {
        {"*", {
            MappingDefinition{"J", "LEFT", {}},
            MappingDefinition{"K", "DOWN", {}},
            MappingDefinition{"I", "UP", {}},
            MappingDefinition{"L", "RIGHT", {}},
            MappingDefinition{"J", "HOME", {"A"}},
            MappingDefinition{"K", "PAGEDOWN", {"A"}},
            MappingDefinition{"I", "PAGEUP", {"A"}},
            MappingDefinition{"L", "END", {"A"}}
        }},
    };
}

ConfigLoader::ModifierSet ConfigLoader::BuildDefaultModifiers() {
    return {"A"};
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
