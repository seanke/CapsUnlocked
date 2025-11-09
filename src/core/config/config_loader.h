#pragma once

#include <map>
#include <string>

namespace caps::core {

// Loads key remap definitions from disk and keeps a normalized copy that the
// rest of the core can query without touching the filesystem again.
class ConfigLoader {
public:
    using MappingTable = std::map<std::string, std::string>;

    ConfigLoader();

    // Reads mappings from the provided path, falling back to defaults when the
    // file is missing. Throws if the file exists but contains invalid syntax.
    void Load(const std::string& path);
    // Re-reads the last successfully loaded file. Useful for hot-reload workflows.
    void Reload();

    [[nodiscard]] const MappingTable& Mappings() const;
    [[nodiscard]] std::string Describe() const;

private:
    [[nodiscard]] MappingTable ParseConfigFile(const std::string& path) const;
    [[nodiscard]] static MappingTable BuildDefaultMappings();
    [[nodiscard]] static std::string NormalizeKeyToken(const std::string& token);
    [[nodiscard]] static std::string Trim(const std::string& value);

    std::string config_path_;
    MappingTable mappings_;
};

} // namespace caps::core
