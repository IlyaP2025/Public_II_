#include <gtest/gtest.h>
#include <QSettings>
#include "settings/settings.h"
#include "common/lighting.h"

using namespace s21;

TEST(SettingsLightTest, LoadEmptyLights) {
    QSettings settings("test", "test");
    settings.clear();
    settings.sync();
    // Принудительно сохраняем пустой список, чтобы перезаписать любые остатки
    Settings::instance().SaveLights({});
    auto lights = Settings::instance().LoadLights();
    EXPECT_TRUE(lights.empty());
}

TEST(SettingsLightTest, SaveAndLoadLights) {
    QSettings settings("test", "test");
    settings.clear();
    settings.sync();
    Settings::instance().SaveLights({}); // начинаем с чистого листа

    std::vector<LightSource> original;
    LightSource l1;
    l1.position = glm::vec3(1.0f, 2.0f, 3.0f);
    l1.diffuse = glm::vec3(0.8f, 0.7f, 0.6f);
    l1.enabled = true;
    original.push_back(l1);
    LightSource l2;
    l2.enabled = false;
    original.push_back(l2);

    Settings::instance().SaveLights(original);
    auto loaded = Settings::instance().LoadLights();

    ASSERT_EQ(loaded.size(), 2u);
    EXPECT_EQ(loaded[0].position, glm::vec3(1.0f, 2.0f, 3.0f));
    EXPECT_EQ(loaded[0].diffuse, glm::vec3(0.8f, 0.7f, 0.6f));
    EXPECT_TRUE(loaded[0].enabled);
    EXPECT_FALSE(loaded[1].enabled);
}
