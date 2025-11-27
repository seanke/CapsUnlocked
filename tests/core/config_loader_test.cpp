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

    fs::path temp_dir_;
};

} // namespace

TEST_F(ConfigLoaderTest, ParsesValidConfigWithNormalization) {
    // New format: app modifier key action
    const fs::path path = WriteConfig("capsunlocked.ini", R"(# comment
      *   *   h   Left
      *   *   j   Down
      chrome   *   k   up
      chrome   A   custom   0x1a
)");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());

    const auto& mappings = loader.Mappings();
    ASSERT_EQ(2u, mappings.size()); // * and CHROME apps
    EXPECT_EQ("LEFT", mappings.at("*").at("*").at("H"));
    EXPECT_EQ("DOWN", mappings.at("*").at("*").at("J"));
    EXPECT_EQ("UP", mappings.at("CHROME").at("*").at("K"));
    EXPECT_EQ("0X1A", mappings.at("CHROME").at("A").at("CUSTOM"));

    const std::string description = loader.Describe();
    EXPECT_NE(std::string::npos, description.find("H -> LEFT"));
    EXPECT_NE(std::string::npos, description.find("CUSTOM -> 0X1A"));
}

TEST_F(ConfigLoaderTest, ReloadRefreshesMappingsFromDisk) {
    // New format: app modifier key action
    const fs::path path = WriteConfig("capsunlocked.ini", "* * h Left\n");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());
    ASSERT_EQ("LEFT", loader.Mappings().at("*").at("*").at("H"));

    WriteConfig("capsunlocked.ini", "* * h Home\n");

    loader.Reload();
    EXPECT_EQ("HOME", loader.Mappings().at("*").at("*").at("H"));
}

TEST_F(ConfigLoaderTest, MissingFileUsesDefaultMappings) {
    const fs::path missing = temp_dir_ / "nope.ini";

    caps::core::ConfigLoader loader;
    loader.Load(missing.string());

    const auto& mappings = loader.Mappings();
    // Default mappings include multiple modifiers: *, A, S, A+S
    ASSERT_EQ(4u, mappings.at("*").size()); // 4 modifiers
    EXPECT_EQ("LEFT", mappings.at("*").at("*").at("H"));
    EXPECT_EQ("DOWN", mappings.at("*").at("*").at("J"));
    EXPECT_EQ("UP", mappings.at("*").at("*").at("K"));
    EXPECT_EQ("RIGHT", mappings.at("*").at("*").at("L"));
    // Check navigation layer (A modifier)
    EXPECT_EQ("END", mappings.at("*").at("A").at("J"));
    EXPECT_EQ("HOME", mappings.at("*").at("A").at("K"));
}

TEST_F(ConfigLoaderTest, MalformedConfigThrows) {
    const fs::path path = WriteConfig("capsunlocked.ini", "invalid_line_without_equals\n");

    caps::core::ConfigLoader loader;
    EXPECT_THROW(loader.Load(path.string()), std::runtime_error);
}

TEST_F(ConfigLoaderTest, ParsesModifierCombinations) {
    // Test that A+S and S+A are normalized to the same key
    const fs::path path = WriteConfig("capsunlocked.ini", R"(
      *   A+S   j   Shift+End
      *   S+A   k   Shift+Home
)");

    caps::core::ConfigLoader loader;
    loader.Load(path.string());

    const auto& mappings = loader.Mappings();
    // Both should be under "A+S" modifier
    EXPECT_EQ("SHIFT+END", mappings.at("*").at("A+S").at("J"));
    EXPECT_EQ("SHIFT+HOME", mappings.at("*").at("A+S").at("K"));
}
