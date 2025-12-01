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
    const fs::path config_path = WriteConfig("* h Left\n");

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

// New tests for modifier support

TEST_F(LayerControllerTest, TracksModifierKeyState) {
    const fs::path config_path = WriteConfig(R"(
[modifiers]
a
s

[maps]
[*] [j] [Down]
)");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine mapping(loader);
    mapping.Initialize();

    caps::core::LayerController controller(mapping);
    controller.OnCapsLockPressed();

    // Initially no modifiers
    EXPECT_TRUE(controller.GetActiveModifiers().empty());

    // Press modifier 'a'
    EXPECT_TRUE(controller.OnKeyEvent({"a", "", true}));
    EXPECT_EQ(1u, controller.GetActiveModifiers().size());
    EXPECT_TRUE(controller.GetActiveModifiers().count("A") > 0);

    // Press modifier 's'
    EXPECT_TRUE(controller.OnKeyEvent({"s", "", true}));
    EXPECT_EQ(2u, controller.GetActiveModifiers().size());
    EXPECT_TRUE(controller.GetActiveModifiers().count("S") > 0);

    // Release modifier 'a'
    EXPECT_TRUE(controller.OnKeyEvent({"a", "", false}));
    EXPECT_EQ(1u, controller.GetActiveModifiers().size());
    EXPECT_FALSE(controller.GetActiveModifiers().count("A") > 0);

    // CapsLock release clears modifiers
    controller.OnCapsLockReleased();
    EXPECT_TRUE(controller.GetActiveModifiers().empty());
}

TEST_F(LayerControllerTest, ModifiersAreSwallowed) {
    const fs::path config_path = WriteConfig(R"(
[modifiers]
a

[maps]
[*] [j] [Down]
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

    // Modifier key press should be swallowed (return true) but not emit action
    EXPECT_TRUE(controller.OnKeyEvent({"a", "", true}));
    EXPECT_TRUE(emitted.empty());

    // Modifier key release should also be swallowed
    EXPECT_TRUE(controller.OnKeyEvent({"a", "", false}));
    EXPECT_TRUE(emitted.empty());
}

TEST_F(LayerControllerTest, MappingActivatesWithModifiers) {
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

    caps::core::MappingEngine mapping(loader);
    mapping.Initialize();

    caps::core::LayerController controller(mapping);

    std::vector<std::pair<std::string, bool>> emitted;
    controller.SetActionCallback(
        [&emitted](const std::string& action, bool pressed) { emitted.emplace_back(action, pressed); });

    controller.OnCapsLockPressed();

    // Without modifiers, 'j' should trigger Down
    EXPECT_TRUE(controller.OnKeyEvent({"j", "", true}));
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("DOWN", emitted[0].first);
    emitted.clear();

    // Press both modifiers
    controller.OnKeyEvent({"a", "", true});
    controller.OnKeyEvent({"s", "", true});

    // Now 'j' should trigger Shift End
    EXPECT_TRUE(controller.OnKeyEvent({"j", "", true}));
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("SHIFT END", emitted[0].first);
    emitted.clear();

    // Release one modifier
    controller.OnKeyEvent({"a", "", false});

    // Now 'j' should fall back to Down
    EXPECT_TRUE(controller.OnKeyEvent({"j", "", true}));
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("DOWN", emitted[0].first);
}

TEST_F(LayerControllerTest, MultipleModifierCombinations) {
    const fs::path config_path = WriteConfig(R"(
[modifiers]
a
s
d

[maps]
[*] [a s d] [j] [Three Mods]
[*] [a s] [j] [Two Mods]
[*] [a] [j] [One Mod]
[*] [] [j] [No Mods]
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

    // Test with all three modifiers
    controller.OnKeyEvent({"a", "", true});
    controller.OnKeyEvent({"s", "", true});
    controller.OnKeyEvent({"d", "", true});
    controller.OnKeyEvent({"j", "", true});
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("THREE MODS", emitted[0].first);
    emitted.clear();

    // Release d, should fall back to two mods
    controller.OnKeyEvent({"d", "", false});
    controller.OnKeyEvent({"j", "", true});
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("TWO MODS", emitted[0].first);
    emitted.clear();

    // Release s, should fall back to one mod
    controller.OnKeyEvent({"s", "", false});
    controller.OnKeyEvent({"j", "", true});
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("ONE MOD", emitted[0].first);
    emitted.clear();

    // Release a, should fall back to no mods
    controller.OnKeyEvent({"a", "", false});
    controller.OnKeyEvent({"j", "", true});
    ASSERT_EQ(1u, emitted.size());
    EXPECT_EQ("NO MODS", emitted[0].first);
}
