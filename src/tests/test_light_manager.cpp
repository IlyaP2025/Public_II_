#include <gtest/gtest.h>
#include "common/lighting.h"
#include "scene/light_manager.h"
#include "commands/light_commands.h"
#include "facade/facade.h"
#include "scene/scene.h"
#include "commands/command_manager.h"

using namespace s21;

// ------------------- LightManager ----------------------
TEST(LightManagerTest, AddLightIncrementsCount) {
    LightManager mgr;
    EXPECT_EQ(mgr.GetLightCount(), 0u);
    size_t idx = mgr.AddLight(LightSource{});
    EXPECT_EQ(idx, 0u);
    EXPECT_EQ(mgr.GetLightCount(), 1u);
}

TEST(LightManagerTest, AddLightThrowsWhenLimitReached) {
    LightManager mgr;
    for (int i = 0; i < 5; ++i) mgr.AddLight(LightSource{});
    EXPECT_THROW(mgr.AddLight(LightSource{}), std::runtime_error);
}

TEST(LightManagerTest, RemoveLightDecrementsCount) {
    LightManager mgr;
    mgr.AddLight(LightSource{});
    mgr.AddLight(LightSource{});
    EXPECT_EQ(mgr.GetLightCount(), 2u);
    mgr.RemoveLight(0);
    EXPECT_EQ(mgr.GetLightCount(), 1u);
}

TEST(LightManagerTest, RemoveLightThrowsOnInvalidIndex) {
    LightManager mgr;
    EXPECT_THROW(mgr.RemoveLight(0), std::out_of_range);
}

TEST(LightManagerTest, UpdateLightChangesParameters) {
    LightManager mgr;
    LightSource l; l.position.x = 1.0f;
    size_t idx = mgr.AddLight(l);
    LightSource nl; nl.position.x = 10.0f;
    mgr.UpdateLight(idx, nl);
    EXPECT_FLOAT_EQ(mgr.GetLight(idx).position.x, 10.0f);
}

TEST(LightManagerTest, GetActiveLightsFiltersDisabled) {
    LightManager mgr;
    LightSource on; on.enabled = true;
    LightSource off; off.enabled = false;
    mgr.AddLight(on);
    mgr.AddLight(off);
    auto active = mgr.GetActiveLights();
    EXPECT_EQ(active.size(), 1u);
}

TEST(LightManagerTest, ClearRemovesAll) {
    LightManager mgr;
    mgr.AddLight(LightSource{});
    mgr.Clear();
    EXPECT_EQ(mgr.GetLightCount(), 0u);
}

TEST(LightManagerTest, ChangeCallbackIsInvoked) {
    LightManager mgr;
    bool called = false;
    mgr.SetChangeCallback([&]() { called = true; });
    mgr.AddLight(LightSource{});
    EXPECT_TRUE(called);
}

// ------------------- Light Commands --------------------
TEST(LightCommandsTest, AddLightCommandWorks) {
    Scene scene; // предполагается, что конструктор создаёт всё необходимое
    auto& lm = scene.GetLightManager();
    EXPECT_EQ(lm.GetLightCount(), 0u);
    AddLightCommand cmd(&scene, LightSource{});
    cmd.execute();
    EXPECT_EQ(lm.GetLightCount(), 1u);
    cmd.undo();
    EXPECT_EQ(lm.GetLightCount(), 0u);
}

TEST(LightCommandsTest, RemoveLightCommandWorks) {
    Scene scene;
    LightSource l;
    scene.GetLightManager().AddLight(l);
    RemoveLightCommand cmd(&scene, 0);
    cmd.execute();
    EXPECT_EQ(scene.GetLightManager().GetLightCount(), 0u);
    cmd.undo();
    EXPECT_EQ(scene.GetLightManager().GetLightCount(), 1u);
}

TEST(LightCommandsTest, UpdateLightCommandWorks) {
    Scene scene;
    LightSource l; l.position.x = 1.0f;
    scene.GetLightManager().AddLight(l);
    LightSource nl; nl.position.x = 100.0f;
    UpdateLightCommand cmd(&scene, 0, nl);
    cmd.execute();
    EXPECT_FLOAT_EQ(scene.GetLightManager().GetLight(0).position.x, 100.0f);
    cmd.undo();
    EXPECT_FLOAT_EQ(scene.GetLightManager().GetLight(0).position.x, 1.0f);
}

// ------------------- Facade Light Methods ---------------
TEST(FacadeLightTest, AddLightViaFacade) {
    CommandManager cmdMgr;
    auto scene = std::make_shared<Scene>();
    Facade facade(scene, &cmdMgr); // если конструктор требует CommandManager*
    EXPECT_EQ(scene->GetLightManager().GetLightCount(), 0u);
    facade.AddLight(LightSource{});
    EXPECT_EQ(scene->GetLightManager().GetLightCount(), 1u);
}

TEST(FacadeLightTest, RemoveLightViaFacade) {
    CommandManager cmdMgr;
    auto scene = std::make_shared<Scene>();
    Facade facade(scene, &cmdMgr);
    scene->GetLightManager().AddLight(LightSource{});
    facade.RemoveLight(0);
    EXPECT_EQ(scene->GetLightManager().GetLightCount(), 0u);
}

TEST(FacadeLightTest, UpdateLightViaFacade) {
    CommandManager cmdMgr;
    auto scene = std::make_shared<Scene>();
    Facade facade(scene, &cmdMgr);
    LightSource l; l.position.x = 5.0f;
    scene->GetLightManager().AddLight(l);
    LightSource nl; nl.position.x = 99.0f;
    facade.UpdateLight(0, nl);
    EXPECT_FLOAT_EQ(scene->GetLightManager().GetLight(0).position.x, 99.0f);
}
