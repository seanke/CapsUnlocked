#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "core/config/config_loader.h"
#include "core/layer/layer_controller.h"
#include "core/mapping/mapping_engine.h"
#include "core/overlay/overlay_model.h"

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
    const fs::path config_path = WriteConfig("h Left\n");

    caps::core::ConfigLoader loader;
    loader.Load(config_path.string());

    caps::core::MappingEngine mapping(loader);
    mapping.Initialize();

    caps::core::OverlayModel overlay;
    overlay.BindMappings(mapping);

    caps::core::LayerController controller(mapping, overlay);

    std::vector<std::pair<std::string, bool>> emitted;
    controller.SetActionCallback(
        [&emitted](const std::string& action, bool pressed) { emitted.emplace_back(action, pressed); });

    // Layer inactive: events pass through untouched.
    EXPECT_FALSE(controller.OnKeyEvent({"H", true}));

    controller.OnCapsLockPressed();
    ASSERT_TRUE(controller.IsLayerActive());

    EXPECT_TRUE(controller.OnKeyEvent({"H", true}));
    EXPECT_TRUE(controller.OnKeyEvent({"H", false}));

    ASSERT_EQ(2u, emitted.size());
    EXPECT_EQ("LEFT", emitted[0].first);
    EXPECT_TRUE(emitted[0].second);
    EXPECT_FALSE(emitted[1].second);

    // Unmapped keys are still swallowed while the layer is active.
    EXPECT_TRUE(controller.OnKeyEvent({"Z", true}));
    EXPECT_EQ(2u, emitted.size());

    controller.OnCapsLockReleased();
    EXPECT_FALSE(controller.IsLayerActive());
    EXPECT_FALSE(controller.OnKeyEvent({"H", true}));
}
