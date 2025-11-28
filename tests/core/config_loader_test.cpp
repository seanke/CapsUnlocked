#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include "core/config/config_loader.h"

namespace fs = std::filesystem;

namespace {

class ConfigLoaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        static int counter = 0;
        temp_dir_ = fs::temp_directory_path() /
                    fs::path("capsunlocked_test_" + std::to_string(++counter));
        fs::create_directories(temp_dir_);
    }

    void TearDown() override {
        fs::remove_all(temp_dir_);
    }

    fs::path WriteConfig(const std::string& name, const std::string& contents) {
        const fs::path path = temp_dir_ / name;
        std::ofstream stream(path);
        stream << contents;
        return path;
    }

    // Helper to find a mapping by source key
    static const caps::core::MappingDefinition* FindMapping(
        const caps::core::ConfigLoader::MappingTable& mappings,
        const std::string& app,
        const std::string& source) {
        auto app_it = mappings.find(app);
        if (app_it == mappings.end()) return nullptr;
        for (const auto& def : app_it->second) {
            if (def.source == source) return &def;
        }
        return nullptr;
    }

    fs::path temp_dir_;
};

} // namespace

TEST_F(ConfigLoaderTest, ParsesValidConfigWithNormalization) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(# comment
      *   h   Left
      *   j   Down
      chrome   k   up
      chrome   custom   0x1a
)");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());

    const auto& mappings = loader.Mappings();
    ASSERT_EQ(2u, mappings.size());
    
    auto h_mapping = FindMapping(mappings, "*", "H");
    ASSERT_NE(nullptr, h_mapping);
    EXPECT_EQ("LEFT", h_mapping->target);
    
    auto j_mapping = FindMapping(mappings, "*", "J");
    ASSERT_NE(nullptr, j_mapping);
    EXPECT_EQ("DOWN", j_mapping->target);
    
    auto k_mapping = FindMapping(mappings, "CHROME", "K");
    ASSERT_NE(nullptr, k_mapping);
    EXPECT_EQ("UP", k_mapping->target);
    
    auto custom_mapping = FindMapping(mappings, "CHROME", "CUSTOM");
    ASSERT_NE(nullptr, custom_mapping);
    EXPECT_EQ("0X1A", custom_mapping->target);

    const std::string description = loader.Describe();
    EXPECT_NE(std::string::npos, description.find("H -> LEFT"));
    EXPECT_NE(std::string::npos, description.find("CUSTOM -> 0X1A"));
}

TEST_F(ConfigLoaderTest, ReloadRefreshesMappingsFromDisk) {
    const fs::path path = WriteConfig("capsunlocked.ini", "* h Left\n");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());
    auto h_mapping = FindMapping(loader.Mappings(), "*", "H");
    ASSERT_NE(nullptr, h_mapping);
    EXPECT_EQ("LEFT", h_mapping->target);

    WriteConfig("capsunlocked.ini", "* h Home\n");

    loader.Reload();
    h_mapping = FindMapping(loader.Mappings(), "*", "H");
    ASSERT_NE(nullptr, h_mapping);
    EXPECT_EQ("HOME", h_mapping->target);
}

TEST_F(ConfigLoaderTest, MissingFileUsesDefaultMappings) {
    const fs::path missing = temp_dir_ / "nope.ini";

    caps::core::ConfigLoader loader;
    loader.Load(missing.string());

    const auto& mappings = loader.Mappings();
    ASSERT_EQ(4u, mappings.at("*").size());
    EXPECT_NE(nullptr, FindMapping(mappings, "*", "H"));
    EXPECT_NE(nullptr, FindMapping(mappings, "*", "J"));
    EXPECT_NE(nullptr, FindMapping(mappings, "*", "K"));
    EXPECT_NE(nullptr, FindMapping(mappings, "*", "L"));
}

TEST_F(ConfigLoaderTest, MalformedConfigThrows) {
    const fs::path path = WriteConfig("capsunlocked.ini", "invalid_line_without_equals\n");

    caps::core::ConfigLoader loader;
    EXPECT_THROW(loader.Load(path.string()), std::runtime_error);
}

// New tests for [modifiers] section

TEST_F(ConfigLoaderTest, ParsesModifiersSection) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
[modifiers]
a
s
Shift

[maps]
[*] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());

    EXPECT_TRUE(loader.HasModifiersSection());
    const auto& modifiers = loader.Modifiers();
    EXPECT_EQ(3u, modifiers.size());
    EXPECT_TRUE(modifiers.count("A") > 0);
    EXPECT_TRUE(modifiers.count("S") > 0);
    EXPECT_TRUE(modifiers.count("SHIFT") > 0);
}

TEST_F(ConfigLoaderTest, ParsesBracketMappingSyntax) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
[maps]
[*] [j] [Down]
[*] [h] [Left]
[chrome] [k] [Up]
)");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());

    const auto& mappings = loader.Mappings();
    auto j_mapping = FindMapping(mappings, "*", "J");
    ASSERT_NE(nullptr, j_mapping);
    EXPECT_EQ("DOWN", j_mapping->target);
    EXPECT_TRUE(j_mapping->required_mods.empty());
}

TEST_F(ConfigLoaderTest, ParsesMappingWithModifiers) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
[modifiers]
a
s

[maps]
[*] [a s] [j] [Shift End]
[*] [] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());

    const auto& mappings = loader.Mappings();
    ASSERT_EQ(1u, mappings.size());
    ASSERT_EQ(2u, mappings.at("*").size());
    
    // Find the mapping with modifiers
    const caps::core::MappingDefinition* mod_mapping = nullptr;
    const caps::core::MappingDefinition* no_mod_mapping = nullptr;
    for (const auto& def : mappings.at("*")) {
        if (def.source == "J") {
            if (!def.required_mods.empty()) {
                mod_mapping = &def;
            } else {
                no_mod_mapping = &def;
            }
        }
    }
    
    ASSERT_NE(nullptr, mod_mapping);
    EXPECT_EQ("SHIFT END", mod_mapping->target);
    EXPECT_EQ(2u, mod_mapping->required_mods.size());
    EXPECT_EQ("A", mod_mapping->required_mods[0]);
    EXPECT_EQ("S", mod_mapping->required_mods[1]);
    
    ASSERT_NE(nullptr, no_mod_mapping);
    EXPECT_EQ("DOWN", no_mod_mapping->target);
    EXPECT_TRUE(no_mod_mapping->required_mods.empty());
}

TEST_F(ConfigLoaderTest, RejectsModifierAsSourceKey) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
[modifiers]
a

[maps]
[*] [a] [Down]
)");

    caps::core::ConfigLoader loader;
    EXPECT_THROW(loader.Load(path.string()), std::runtime_error);
}

TEST_F(ConfigLoaderTest, RejectsModifierAsTargetKey) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
[modifiers]
a

[maps]
[*] [j] [a]
)");

    caps::core::ConfigLoader loader;
    EXPECT_THROW(loader.Load(path.string()), std::runtime_error);
}

TEST_F(ConfigLoaderTest, RejectsUndefinedModifierInMapping) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
[modifiers]
a

[maps]
[*] [a s] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    EXPECT_THROW(loader.Load(path.string()), std::runtime_error);
}

TEST_F(ConfigLoaderTest, AllowsModifiersWithoutModifiersSection) {
    // Legacy behavior: when no [modifiers] section exists, any modifier list is allowed
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
[maps]
[*] [a s] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    // This should not throw - backwards compatible
    EXPECT_NO_THROW(loader.Load(path.string()));
    EXPECT_FALSE(loader.HasModifiersSection());
}

TEST_F(ConfigLoaderTest, BackwardsCompatibleWithLegacyFormat) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
*   h   Left
*   j   Down
)");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());

    EXPECT_FALSE(loader.HasModifiersSection());
    const auto& mappings = loader.Mappings();
    EXPECT_NE(nullptr, FindMapping(mappings, "*", "H"));
    EXPECT_NE(nullptr, FindMapping(mappings, "*", "J"));
}

TEST_F(ConfigLoaderTest, DescribeIncludesModifiers) {
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
[modifiers]
a
s

[maps]
[*] [a s] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());

    const std::string description = loader.Describe();
    EXPECT_NE(std::string::npos, description.find("2 modifiers"));
    EXPECT_NE(std::string::npos, description.find("[A S]"));
}
