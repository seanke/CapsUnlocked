#pragma once

#include <map>
#include <string>

namespace caps::core {

class ConfigLoader {
public:
    using MappingTable = std::map<std::string, std::string>;

    ConfigLoader();

    void Load(const std::string& path);
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
