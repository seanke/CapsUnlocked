#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <set>

#include "core/config/config_loader.h"
#include "core/mapping/mapping_engine.h"

namespace fs = std::filesystem;

namespace {

class MappingEngineTest : public ::testing::Test {
protected:
    void SetUp() override {
        static int counter = 0;
        temp_dir_ = fs::temp_directory_path() /
                    fs::path("capsunlocked_mapping_test_" + std::to_string(++counter));
        fs::create_directories(temp_dir_);
    }

    void TearDown() override {
        fs::remove_all(temp_dir_);
    }

    fs::path WriteConfig(const std::string& contents) {
        const fs::path path = temp_dir_ / "capsunlocked.ini";
        std::ofstream stream(path);
        stream << contents;
        return path;
    }

    fs::path temp_dir_;
};

} // namespace

TEST_F(MappingEngineTest, ResolvesAndReloadsMappings) {
    const fs::path config_path = WriteConfig("* h Left\n* j Down\n");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    auto left = engine.ResolveMapping("h", "");
    ASSERT_TRUE(left.has_value());
    EXPECT_EQ("LEFT", left->action);
    EXPECT_EQ("*", left->app);

    auto down = engine.ResolveMapping("J", "");
    ASSERT_TRUE(down.has_value());
    EXPECT_EQ("DOWN", down->action);
    EXPECT_EQ("*", down->app);

    // Update the config on disk and ensure the engine refreshes its cache.
    WriteConfig("* l Right\n");
    loader.Reload();
    engine.UpdateFromConfig();

    EXPECT_FALSE(engine.ResolveMapping("H", "").has_value());
    auto right = engine.ResolveMapping("l", "");
    ASSERT_TRUE(right.has_value());
    EXPECT_EQ("RIGHT", right->action);
    EXPECT_EQ("*", right->app);

    const auto entries = engine.EnumerateMappings();
    ASSERT_EQ(1u, entries.size());
    EXPECT_EQ("*", entries.front().app);
    EXPECT_EQ("L", entries.front().source);
    EXPECT_EQ("RIGHT", entries.front().target);
}

// New tests for modifier support

TEST_F(MappingEngineTest, ResolvesWithModifiers) {
    const fs::path config_path = WriteConfig(R"(
[modifiers]
a
s

[maps]
[*] [a s] [j] [Shift End]
[*] [] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    // Without modifiers, should get Down
    std::set<std::string> no_mods;
    auto result = engine.ResolveMapping("j", "", no_mods);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("DOWN", result->action);
    EXPECT_TRUE(result->required_mods.empty());

    // With both modifiers, should get Shift End
    std::set<std::string> both_mods = {"A", "S"};
    result = engine.ResolveMapping("j", "", both_mods);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("SHIFT END", result->action);
    EXPECT_EQ(2u, result->required_mods.size());

    // With only one modifier, should get Down (fallback)
    std::set<std::string> one_mod = {"A"};
    result = engine.ResolveMapping("j", "", one_mod);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("DOWN", result->action);
}

TEST_F(MappingEngineTest, IsModifier) {
    const fs::path config_path = WriteConfig(R"(
[modifiers]
a
s

[maps]
[*] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    EXPECT_TRUE(engine.IsModifier("a"));
    EXPECT_TRUE(engine.IsModifier("A"));
    EXPECT_TRUE(engine.IsModifier("s"));
    EXPECT_FALSE(engine.IsModifier("j"));
    EXPECT_FALSE(engine.IsModifier("h"));
}

TEST_F(MappingEngineTest, GetModifiers) {
    const fs::path config_path = WriteConfig(R"(
[modifiers]
a
s
Ctrl

[maps]
[*] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    const auto& mods = engine.GetModifiers();
    EXPECT_EQ(3u, mods.size());
    EXPECT_TRUE(mods.count("A") > 0);
    EXPECT_TRUE(mods.count("S") > 0);
    EXPECT_TRUE(mods.count("CTRL") > 0);
}

TEST_F(MappingEngineTest, PrefersMoreSpecificMapping) {
    const fs::path config_path = WriteConfig(R"(
[modifiers]
a
s
d

[maps]
[*] [a s d] [j] [Most Specific]
[*] [a s] [j] [Less Specific]
[*] [a] [j] [Even Less Specific]
[*] [] [j] [No Modifiers]
)");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    // With all three modifiers, should get most specific
    std::set<std::string> all_mods = {"A", "S", "D"};
    auto result = engine.ResolveMapping("j", "", all_mods);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("MOST SPECIFIC", result->action);

    // With two modifiers, should get less specific
    std::set<std::string> two_mods = {"A", "S"};
    result = engine.ResolveMapping("j", "", two_mods);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("LESS SPECIFIC", result->action);

    // With one modifier, should get even less specific
    std::set<std::string> one_mod = {"A"};
    result = engine.ResolveMapping("j", "", one_mod);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("EVEN LESS SPECIFIC", result->action);

    // With no modifiers, should get the base mapping
    std::set<std::string> no_mods;
    result = engine.ResolveMapping("j", "", no_mods);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ("NO MODIFIERS", result->action);
}

TEST_F(MappingEngineTest, EnumerateMappingsIncludesModifiers) {
    const fs::path config_path = WriteConfig(R"(
[modifiers]
a

[maps]
[*] [a] [j] [ShiftEnd]
[*] [] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    const auto entries = engine.EnumerateMappings();
    EXPECT_EQ(2u, entries.size());
    
    // Check that entries with modifiers come first (more specific)
    EXPECT_EQ("J", entries[0].source);
    EXPECT_EQ(1u, entries[0].required_mods.size());
    EXPECT_EQ("A", entries[0].required_mods[0]);
}
