#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "core/config/config_loader.h"
#include "core/layer/layer_controller.h"
#include "core/mapping/mapping_engine.h"

namespace fs = std::filesystem;

namespace {

class LayerControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        static int counter = 0;
        temp_dir_ = fs::temp_directory_path() /
                    fs::path("capsunlocked_layer_test_" + std::to_string(++counter));
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

TEST_F(LayerControllerTest, DispatchesMappedActionsWhileLayerActive) {
    // New format: app modifier key action
    const fs::path config_path = WriteConfig("* * h Left\n");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine mapping(loader);
    mapping.Initialize();

    caps::core::LayerController controller(mapping);

    std::vector<std::pair<std::string, bool>> emitted;
    controller.SetActionCallback(
        [&emitted](const std::string& action, bool pressed) { emitted.emplace_back(action, pressed); });

    // Layer inactive: events pass through untouched.
    EXPECT_FALSE(controller.OnKeyEvent({"H", "", true}));

    controller.OnCapsLockPressed();
    ASSERT_TRUE(controller.IsLayerActive());

    EXPECT_TRUE(controller.OnKeyEvent({"H", "", true}));
    EXPECT_TRUE(controller.OnKeyEvent({"H", "", false}));

    ASSERT_EQ(2u, emitted.size());
    EXPECT_EQ("LEFT", emitted[0].first);
    EXPECT_TRUE(emitted[0].second);
    EXPECT_FALSE(emitted[1].second);

    // Unmapped keys are still swallowed while the layer is active.
    EXPECT_TRUE(controller.OnKeyEvent({"Z", "", true}));
    EXPECT_EQ(2u, emitted.size());

    controller.OnCapsLockReleased();
    EXPECT_FALSE(controller.IsLayerActive());
    EXPECT_FALSE(controller.OnKeyEvent({"H", "", true}));
}

TEST_F(LayerControllerTest, TracksModifierKeysAndDispatchesCorrectMappings) {
    // Config with multiple modifier combinations
    const fs::path config_path = WriteConfig(R"(
        * * j Down
        * A j End
        * S j Shift+Down
        * A+S j Shift+End
    )");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine mapping(loader);
    mapping.Initialize();

    caps::core::LayerController controller(mapping);

    std::vector<std::pair<std::string, bool>> emitted;
    controller.SetActionCallback(
        [&emitted](const std::string& action, bool pressed) { emitted.emplace_back(action, pressed); });

    controller.OnCapsLockPressed();
    ASSERT_TRUE(controller.IsLayerActive());

    // Basic mapping without modifiers
    emitted.clear();
    EXPECT_TRUE(controller.OnKeyEvent({"J", "", true}));
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("DOWN", emitted[0].first);

    // Hold A modifier, then press J
    emitted.clear();
    EXPECT_TRUE(controller.OnKeyEvent({"A", "", true})); // Press A
    EXPECT_TRUE(controller.GetActiveModifiers().count("A") > 0);
    EXPECT_TRUE(controller.OnKeyEvent({"J", "", true})); // Press J with A held
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("END", emitted[0].first);
    EXPECT_TRUE(controller.OnKeyEvent({"A", "", false})); // Release A

    // Hold S modifier, then press J
    emitted.clear();
    EXPECT_TRUE(controller.OnKeyEvent({"S", "", true})); // Press S
    EXPECT_TRUE(controller.GetActiveModifiers().count("S") > 0);
    EXPECT_TRUE(controller.OnKeyEvent({"J", "", true})); // Press J with S held
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("SHIFT+DOWN", emitted[0].first);
    EXPECT_TRUE(controller.OnKeyEvent({"S", "", false})); // Release S

    // Hold both A and S modifiers, then press J
    emitted.clear();
    EXPECT_TRUE(controller.OnKeyEvent({"A", "", true})); // Press A
    EXPECT_TRUE(controller.OnKeyEvent({"S", "", true})); // Press S
    EXPECT_TRUE(controller.GetActiveModifiers().count("A") > 0);
    EXPECT_TRUE(controller.GetActiveModifiers().count("S") > 0);
    EXPECT_TRUE(controller.OnKeyEvent({"J", "", true})); // Press J with A+S held
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("SHIFT+END", emitted[0].first);

    controller.OnCapsLockReleased();
    EXPECT_FALSE(controller.IsLayerActive());
    EXPECT_TRUE(controller.GetActiveModifiers().empty());
}

TEST_F(LayerControllerTest, ModifierKeysSwallowedWhileLayerActive) {
    const fs::path config_path = WriteConfig("* * h Left\n");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine mapping(loader);
    mapping.Initialize();

    caps::core::LayerController controller(mapping);

    std::vector<std::pair<std::string, bool>> emitted;
    controller.SetActionCallback(
        [&emitted](const std::string& action, bool pressed) { emitted.emplace_back(action, pressed); });

    controller.OnCapsLockPressed();

    // Modifier keys (A, S) should be swallowed and not emit any action
    EXPECT_TRUE(controller.OnKeyEvent({"A", "", true}));
    EXPECT_TRUE(controller.OnKeyEvent({"S", "", true}));
    EXPECT_TRUE(controller.OnKeyEvent({"A", "", false}));
    EXPECT_TRUE(controller.OnKeyEvent({"S", "", false}));

    // No actions should have been emitted for modifier keys
    EXPECT_EQ(0u, emitted.size());
}
