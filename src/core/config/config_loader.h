#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

namespace caps::core {

// Represents a single key mapping with optional modifier requirements.
struct MappingDefinition {
    std::string source;                       // Normalized source key
    std::string target;                       // Normalized target key(s)
    std::vector<std::string> required_mods;   // Modifiers that must be held (logical AND)
};

// Loads key remap definitions from disk and keeps a normalized copy that the
// rest of the core can query without touching the filesystem again.
class ConfigLoader {
public:
    // app -> vector of mapping definitions (ordered for priority)
    using MappingTable = std::map<std::string, std::vector<MappingDefinition>>;
    using ModifierSet = std::set<std::string>;

    ConfigLoader();

    // Reads mappings from the provided path, falling back to defaults when the
    // file is missing. Throws if the file exists but contains invalid syntax.
    void Load(const std::string& path);
    // Re-reads the last successfully loaded file. Useful for hot-reload workflows.
    void Reload();

    [[nodiscard]] const MappingTable& Mappings() const;
    [[nodiscard]] const ModifierSet& Modifiers() const;
    [[nodiscard]] bool HasModifiersSection() const;
    [[nodiscard]] std::string Describe() const;

    // Expose normalization utilities for external use
    [[nodiscard]] static std::string NormalizeKeyToken(const std::string& token);
    [[nodiscard]] static std::string NormalizeAppToken(const std::string& token);
    [[nodiscard]] static std::string Trim(const std::string& value);

private:
    struct ParseResult {
        MappingTable mappings;
        ModifierSet modifiers;
        bool has_modifiers_section{false};
    };

    [[nodiscard]] ParseResult ParseConfigFile(const std::string& path) const;
    [[nodiscard]] static MappingTable BuildDefaultMappings();
    [[nodiscard]] static ModifierSet BuildDefaultModifiers();

    std::string config_path_;
    MappingTable mappings_;
    ModifierSet modifiers_;
    bool has_modifiers_section_{false};
};

} // namespace caps::core
