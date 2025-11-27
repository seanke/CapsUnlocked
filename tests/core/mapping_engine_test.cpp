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
    // New format: app modifier key action
    const fs::path config_path = WriteConfig("* * h Left\n* * j Down\n");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    auto left = engine.ResolveMapping("h", "", {});
    ASSERT_TRUE(left.has_value());
    EXPECT_EQ("LEFT", left->action);
    EXPECT_EQ("*", left->app);
    EXPECT_EQ("*", left->modifier);

    auto down = engine.ResolveMapping("J", "", {});
    ASSERT_TRUE(down.has_value());
    EXPECT_EQ("DOWN", down->action);
    EXPECT_EQ("*", down->app);

    // Update the config on disk and ensure the engine refreshes its cache.
    WriteConfig("* * l Right\n");
    loader.Reload();
    engine.UpdateFromConfig();

    EXPECT_FALSE(engine.ResolveMapping("H", "", {}).has_value());
    auto right = engine.ResolveMapping("l", "", {});
    ASSERT_TRUE(right.has_value());
    EXPECT_EQ("RIGHT", right->action);
    EXPECT_EQ("*", right->app);

    const auto entries = engine.EnumerateMappings();
    ASSERT_EQ(1u, entries.size());
    EXPECT_EQ("*", entries.front().app);
    EXPECT_EQ("*", entries.front().modifier);
    EXPECT_EQ("L", entries.front().source);
    EXPECT_EQ("RIGHT", entries.front().target);
}

TEST_F(MappingEngineTest, ResolvesWithModifiers) {
    // Test different modifier combinations
    const fs::path config_path = WriteConfig(R"(
        * * j Down
        * A j End
        * S j Shift+Down
        * A+S j Shift+End
    )");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    // No modifier
    auto basic = engine.ResolveMapping("j", "", {});
    ASSERT_TRUE(basic.has_value());
    EXPECT_EQ("DOWN", basic->action);
    EXPECT_EQ("*", basic->modifier);

    // A modifier
    auto with_a = engine.ResolveMapping("j", "", {"A"});
    ASSERT_TRUE(with_a.has_value());
    EXPECT_EQ("END", with_a->action);
    EXPECT_EQ("A", with_a->modifier);

    // S modifier
    auto with_s = engine.ResolveMapping("j", "", {"S"});
    ASSERT_TRUE(with_s.has_value());
    EXPECT_EQ("SHIFT+DOWN", with_s->action);
    EXPECT_EQ("S", with_s->modifier);

    // A+S modifiers
    auto with_as = engine.ResolveMapping("j", "", {"A", "S"});
    ASSERT_TRUE(with_as.has_value());
    EXPECT_EQ("SHIFT+END", with_as->action);
    EXPECT_EQ("A+S", with_as->modifier);

    // Order shouldn't matter
    auto with_sa = engine.ResolveMapping("j", "", {"S", "A"});
    ASSERT_TRUE(with_sa.has_value());
    EXPECT_EQ("SHIFT+END", with_sa->action);
    EXPECT_EQ("A+S", with_sa->modifier);
}

TEST_F(MappingEngineTest, UnknownModifierReturnsNoMatch) {
    const fs::path config_path = WriteConfig("* * j Down\n");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    // With unknown modifier, should not find mapping
    auto result = engine.ResolveMapping("j", "", {"X"});
    EXPECT_FALSE(result.has_value());
}
