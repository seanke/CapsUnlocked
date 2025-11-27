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

    struct EmittedAction {
        std::string action;
        bool pressed;
        caps::core::Modifiers modifiers;
    };
    std::vector<EmittedAction> emitted;
    controller.SetActionCallback(
        [&emitted](const std::string& action, bool pressed, caps::core::Modifiers modifiers) {
            emitted.push_back({action, pressed, modifiers});
        });

    // Layer inactive: events pass through untouched.
    EXPECT_FALSE(controller.OnKeyEvent({"H", "", true}));

    controller.OnCapsLockPressed();
    ASSERT_TRUE(controller.IsLayerActive());

    EXPECT_TRUE(controller.OnKeyEvent({"H", "", true}));
    EXPECT_TRUE(controller.OnKeyEvent({"H", "", false}));

    ASSERT_EQ(2u, emitted.size());
    EXPECT_EQ("LEFT", emitted[0].action);
    EXPECT_TRUE(emitted[0].pressed);
    EXPECT_FALSE(emitted[1].pressed);

    // Unmapped keys are still swallowed while the layer is active.
    EXPECT_TRUE(controller.OnKeyEvent({"Z", "", true}));
    EXPECT_EQ(2u, emitted.size());

    controller.OnCapsLockReleased();
    EXPECT_FALSE(controller.IsLayerActive());
    EXPECT_FALSE(controller.OnKeyEvent({"H", "", true}));
}

TEST_F(LayerControllerTest, PreservesModifiersInMappedActions) {
    const fs::path config_path = WriteConfig("* l Right\n");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine mapping(loader);
    mapping.Initialize();

    caps::core::LayerController controller(mapping);

    struct EmittedAction {
        std::string action;
        bool pressed;
        caps::core::Modifiers modifiers;
    };
    std::vector<EmittedAction> emitted;
    controller.SetActionCallback(
        [&emitted](const std::string& action, bool pressed, caps::core::Modifiers modifiers) {
            emitted.push_back({action, pressed, modifiers});
        });

    controller.OnCapsLockPressed();
    ASSERT_TRUE(controller.IsLayerActive());

    // Simulate Ctrl+L while CapsLock is held - should emit Ctrl+Right
    caps::core::KeyEvent ctrl_l_down{"L", "", true, caps::core::Modifiers::Control};
    EXPECT_TRUE(controller.OnKeyEvent(ctrl_l_down));

    caps::core::KeyEvent ctrl_l_up{"L", "", false, caps::core::Modifiers::Control};
    EXPECT_TRUE(controller.OnKeyEvent(ctrl_l_up));

    ASSERT_EQ(2u, emitted.size());
    EXPECT_EQ("RIGHT", emitted[0].action);
    EXPECT_TRUE(emitted[0].pressed);
    EXPECT_TRUE(caps::core::HasModifier(emitted[0].modifiers, caps::core::Modifiers::Control));
    
    EXPECT_EQ("RIGHT", emitted[1].action);
    EXPECT_FALSE(emitted[1].pressed);
    EXPECT_TRUE(caps::core::HasModifier(emitted[1].modifiers, caps::core::Modifiers::Control));

    controller.OnCapsLockReleased();
}
