#include <gtest/gtest.h>

#include "commands/command_manager.h"
#include "common/lighting.h"
#include "facade/facade.h"
#include "scene/mesh.h"
#include "scene/scene.h"

using namespace s21;

class FacadeTest : public ::testing::Test {
 protected:
  void SetUp() override {
    scene_ = std::make_shared<Scene>();
    facade_ = std::make_unique<Facade>(scene_);
    // Добавляем тестовый объект для операций трансформации
    auto mesh = std::make_unique<Mesh>();
    mesh->SetName("test_mesh");
    scene_->AddObject(std::move(mesh));
    scene_->SetSelected({scene_->GetObjects().front().get()});
    // Переводим фасад в состояние Ready, чтобы CanTransform() и др. возвращали
    // true
    facade_->NotifyLoadStarted();
    facade_->NotifyLoadFinished(true, "");
  }

  std::shared_ptr<Scene> scene_;
  std::unique_ptr<Facade> facade_;
};

TEST_F(FacadeTest, MoveSelected) {
  Point delta{1.0f, 0.0f, 0.0f};
  facade_->MoveSelected(delta);
  auto pos = scene_->GetSelected().front()->GetTransform().GetPosition();
  EXPECT_FLOAT_EQ(pos.x, 1.0f);
}

TEST_F(FacadeTest, RotateSelected) {
  Point delta{0.0f, 90.0f, 0.0f};
  facade_->RotateSelected(delta);
  auto rot = scene_->GetSelected().front()->GetTransform().GetRotation();
  EXPECT_FLOAT_EQ(rot.y, 90.0f);
}

TEST_F(FacadeTest, ScaleSelected) {
  Point delta{1.0f, 0.0f, 0.0f};
  facade_->ScaleSelected(delta);
  auto scale = scene_->GetSelected().front()->GetTransform().GetScale();
  EXPECT_FLOAT_EQ(scale.x, 2.0f);
}

TEST_F(FacadeTest, UndoRedo) {
  Point delta{5.0f, 0.0f, 0.0f};
  facade_->MoveSelected(delta);
  facade_->Undo();
  auto pos = scene_->GetSelected().front()->GetTransform().GetPosition();
  EXPECT_FLOAT_EQ(pos.x, 0.0f);
  facade_->Redo();
  pos = scene_->GetSelected().front()->GetTransform().GetPosition();
  EXPECT_FLOAT_EQ(pos.x, 5.0f);
}

TEST_F(FacadeTest, CanUndoRedo) {
  EXPECT_FALSE(facade_->CanUndo());
  facade_->MoveSelected({1, 0, 0});
  EXPECT_TRUE(facade_->CanUndo());
  facade_->Undo();
  EXPECT_FALSE(facade_->CanUndo());
  EXPECT_TRUE(facade_->CanRedo());
}

TEST_F(FacadeTest, StateMethods) {
  // Теперь мы в состоянии Ready
  EXPECT_TRUE(facade_->CanTransform());
  EXPECT_FALSE(facade_->CanLoadModel());  // Не Idle
  EXPECT_TRUE(facade_->CanStartGifRecording());
  EXPECT_TRUE(facade_->CanSaveScreenshot());
  EXPECT_TRUE(facade_->CanResetCamera());
}

TEST_F(FacadeTest, NotifyMethods) {
  EXPECT_NO_THROW(facade_->NotifyLoadStarted());
  EXPECT_NO_THROW(facade_->NotifyGifRecordingStarted());
  EXPECT_NO_THROW(facade_->NotifyGifRecordingFinished());
  EXPECT_NO_THROW(facade_->NotifyModelCleared());
}

TEST_F(FacadeTest, GetLights) {
  auto lights = facade_->GetLights();
  EXPECT_TRUE(lights.empty());
  facade_->AddLight(LightSource{});
  lights = facade_->GetLights();
  EXPECT_EQ(lights.size(), 1u);
}

TEST_F(FacadeTest, ResetErrorAndGetErrorMessage) {
  std::string err = facade_->GetErrorMessage();
  EXPECT_TRUE(err.empty());
  facade_->ResetError();
}
