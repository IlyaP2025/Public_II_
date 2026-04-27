#include <gtest/gtest.h>

#include <memory>

#include "commands/command_manager.h"
#include "commands/macro_command.h"
#include "commands/transform_commands.h"
#include "scene/mesh.h"
#include "scene/scene.h"

using namespace s21;

class CommandTest : public ::testing::Test {
 protected:
  void SetUp() override {
    scene = std::make_unique<Scene>();
    mesh = std::make_unique<Mesh>();
    rawMesh = mesh.get();
    scene->AddObject(std::move(mesh));
    // изначально выделим этот объект
    scene->SetSelected({rawMesh});
  }

  std::unique_ptr<Scene> scene;
  std::unique_ptr<Mesh> mesh;
  Mesh* rawMesh;
};

TEST_F(CommandTest, MoveCommand) {
  Point oldPos = rawMesh->GetTransform().GetPosition();
  Point newPos(1, 2, 3);
  MoveCommand cmd(scene.get(), rawMesh, newPos);
  cmd.execute();
  Point posAfter = rawMesh->GetTransform().GetPosition();
  EXPECT_EQ(posAfter.x, newPos.x);
  EXPECT_EQ(posAfter.y, newPos.y);
  EXPECT_EQ(posAfter.z, newPos.z);

  cmd.undo();
  Point posAfterUndo = rawMesh->GetTransform().GetPosition();
  EXPECT_EQ(posAfterUndo.x, oldPos.x);
  EXPECT_EQ(posAfterUndo.y, oldPos.y);
  EXPECT_EQ(posAfterUndo.z, oldPos.z);
}

TEST_F(CommandTest, RotateCommand) {
  Point oldRot = rawMesh->GetTransform().GetRotation();
  Point newRot(30, 45, 60);
  RotateCommand cmd(scene.get(), rawMesh, newRot);
  cmd.execute();
  Point rotAfter = rawMesh->GetTransform().GetRotation();
  EXPECT_EQ(rotAfter.x, newRot.x);
  EXPECT_EQ(rotAfter.y, newRot.y);
  EXPECT_EQ(rotAfter.z, newRot.z);

  cmd.undo();
  Point rotAfterUndo = rawMesh->GetTransform().GetRotation();
  EXPECT_EQ(rotAfterUndo.x, oldRot.x);
  EXPECT_EQ(rotAfterUndo.y, oldRot.y);
  EXPECT_EQ(rotAfterUndo.z, oldRot.z);
}

TEST_F(CommandTest, ScaleCommand) {
  Point oldScale = rawMesh->GetTransform().GetScale();
  Point newScale(2, 2, 2);
  ScaleCommand cmd(scene.get(), rawMesh, newScale);
  cmd.execute();
  Point scaleAfter = rawMesh->GetTransform().GetScale();
  EXPECT_EQ(scaleAfter.x, newScale.x);
  EXPECT_EQ(scaleAfter.y, newScale.y);
  EXPECT_EQ(scaleAfter.z, newScale.z);

  cmd.undo();
  Point scaleAfterUndo = rawMesh->GetTransform().GetScale();
  EXPECT_EQ(scaleAfterUndo.x, oldScale.x);
  EXPECT_EQ(scaleAfterUndo.y, oldScale.y);
  EXPECT_EQ(scaleAfterUndo.z, oldScale.z);
}

TEST_F(CommandTest, CommandManagerSingle) {
  CommandManager mgr;
  Point oldPos = rawMesh->GetTransform().GetPosition();
  Point newPos(5, 5, 5);
  auto cmd = std::make_unique<MoveCommand>(scene.get(), rawMesh, newPos);

  mgr.executeCommand(std::move(cmd));
  Point posAfter = rawMesh->GetTransform().GetPosition();
  EXPECT_EQ(posAfter.x, newPos.x);

  mgr.undo();
  Point posAfterUndo = rawMesh->GetTransform().GetPosition();
  EXPECT_EQ(posAfterUndo.x, oldPos.x);

  mgr.redo();
  Point posAfterRedo = rawMesh->GetTransform().GetPosition();
  EXPECT_EQ(posAfterRedo.x, newPos.x);
}

TEST_F(CommandTest, CommandManagerMacro) {
  CommandManager mgr;
  auto macro = std::make_unique<MacroCommand>();
  Point oldPos = rawMesh->GetTransform().GetPosition();
  Point newPos1(1, 0, 0);
  Point newPos2(0, 2, 0);
  macro->addCommand(
      std::make_unique<MoveCommand>(scene.get(), rawMesh, newPos1));
  macro->addCommand(
      std::make_unique<MoveCommand>(scene.get(), rawMesh, newPos2));

  mgr.executeCommand(std::move(macro));
  Point posAfter = rawMesh->GetTransform().GetPosition();
  EXPECT_EQ(posAfter.x, newPos2.x);
  EXPECT_EQ(posAfter.y, newPos2.y);

  mgr.undo();
  Point posAfterUndo = rawMesh->GetTransform().GetPosition();
  EXPECT_EQ(posAfterUndo.x, oldPos.x);
  EXPECT_EQ(posAfterUndo.y, oldPos.y);
  EXPECT_EQ(posAfterUndo.z, oldPos.z);
}

TEST(CommandManagerTest, CanUndoRedo) {
  CommandManager mgr;
  EXPECT_FALSE(mgr.canUndo());
  EXPECT_FALSE(mgr.canRedo());

  auto mockScene = std::make_unique<Scene>();
  auto mesh = std::make_unique<Mesh>();
  Mesh* raw = mesh.get();
  mockScene->AddObject(std::move(mesh));
  auto cmd =
      std::make_unique<MoveCommand>(mockScene.get(), raw, Point(1, 1, 1));
  mgr.executeCommand(std::move(cmd));
  EXPECT_TRUE(mgr.canUndo());
  EXPECT_FALSE(mgr.canRedo());

  mgr.undo();
  EXPECT_TRUE(mgr.canRedo());
  EXPECT_FALSE(mgr.canUndo());  // после undo можно redo, но undo уже пусто?
                                // зависит от реализации: в нашем менеджере
                                // после undo команда перемещается в redoStack,
                                // так что undoStack пуст, redoStack не пуст.
                                // canUndo должен быть false, canRedo true.
  EXPECT_FALSE(mgr.canUndo());
  EXPECT_TRUE(mgr.canRedo());

  mgr.redo();
  EXPECT_TRUE(mgr.canUndo());
  EXPECT_FALSE(mgr.canRedo());
}

TEST(CommandManagerTest, Clear) {
  CommandManager mgr;
  auto mockScene = std::make_unique<Scene>();
  auto mesh = std::make_unique<Mesh>();
  Mesh* raw = mesh.get();
  mockScene->AddObject(std::move(mesh));
  auto cmd =
      std::make_unique<MoveCommand>(mockScene.get(), raw, Point(1, 1, 1));
  mgr.executeCommand(std::move(cmd));
  mgr.clear();
  EXPECT_FALSE(mgr.canUndo());
  EXPECT_FALSE(mgr.canRedo());
}
