#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

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
    const fs::path config_path = WriteConfig("h=Left\nj=Down\n");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine engine(loader);
    engine.Initialize();

    auto left = engine.ResolveMapping("h");
    ASSERT_TRUE(left.has_value());
    EXPECT_EQ("LEFT", *left);

    auto down = engine.ResolveMapping("J");
    ASSERT_TRUE(down.has_value());
    EXPECT_EQ("DOWN", *down);

    // Update the config on disk and ensure the engine refreshes its cache.
    WriteConfig("l=Right\n");
    loader.Reload();
    engine.UpdateFromConfig();

    EXPECT_FALSE(engine.ResolveMapping("H").has_value());
    auto right = engine.ResolveMapping("l");
    ASSERT_TRUE(right.has_value());
    EXPECT_EQ("RIGHT", *right);

    const auto entries = engine.EnumerateMappings();
    ASSERT_EQ(1u, entries.size());
    EXPECT_EQ("L", entries.front().first);
    EXPECT_EQ("RIGHT", entries.front().second);
}
