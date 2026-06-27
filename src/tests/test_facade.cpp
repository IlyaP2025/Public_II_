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
    auto mesh = std::make_unique<Mesh>();
    mesh->SetName("test_mesh");
    scene_->AddObject(std::move(mesh));
    scene_->SetSelected({scene_->GetObjects().front().get()});
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
  EXPECT_TRUE(facade_->CanTransform());
  EXPECT_FALSE(facade_->CanLoadModel());
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

TEST_F(FacadeTest, LoadModelBlocked) {
  EXPECT_NO_THROW(facade_->LoadModel());
}

TEST_F(FacadeTest, MacroCommandMoveMultiple) {
  auto mesh2 = std::make_unique<Mesh>();
  mesh2->SetName("mesh2");
  scene_->AddObject(std::move(mesh2));
  scene_->SetSelected({scene_->GetObjects()[0].get(), scene_->GetObjects()[1].get()});
  facade_->MoveSelected(Point{1, 0, 0});
  auto pos0 = scene_->GetObjects()[0]->GetTransform().GetPosition();
  auto pos1 = scene_->GetObjects()[1]->GetTransform().GetPosition();
  EXPECT_FLOAT_EQ(pos0.x, 1.0f);
  EXPECT_FLOAT_EQ(pos1.x, 1.0f);
}

TEST_F(FacadeTest, AddStateObserver) {
  bool called = false;
  facade_->AddStateObserver([&](AppState) { called = true; });
  facade_->NotifyModelCleared();
  EXPECT_TRUE(called);
}

TEST_F(FacadeTest, GetState) {
  EXPECT_EQ(facade_->GetState(), AppState::Ready);
}

// StateMachineTest
TEST(StateMachineTest, ErrorToIdle) {
  AppStateMachine sm;
  EXPECT_EQ(sm.GetState(), AppState::Idle);
  sm.OnLoadStarted();
  EXPECT_EQ(sm.GetState(), AppState::Loading);
  sm.OnLoadFinished(false, "error");
  EXPECT_EQ(sm.GetState(), AppState::Error);
  sm.OnResetError();
  EXPECT_EQ(sm.GetState(), AppState::Idle);
}

// Проверка, что трансформации и undo/redo не выполняются, когда состояние не Ready
TEST_F(FacadeTest, BlockedOperations) {
  facade_->NotifyModelCleared();               // переводим в Idle, CanTransform == false
  Point oldPos = scene_->GetSelected().front()->GetTransform().GetPosition();
  facade_->MoveSelected({1, 0, 0});
  facade_->RotateSelected({1, 0, 0});
  facade_->ScaleSelected({1, 0, 0});
  facade_->Undo();
  facade_->Redo();
  auto pos = scene_->GetSelected().front()->GetTransform().GetPosition();
  EXPECT_FLOAT_EQ(pos.x, oldPos.x) << "Position must not change when blocked";
}

// Вызов NotifyLoadFinished, когда автомат не в Loading
TEST_F(FacadeTest, NotifyLoadFinishedWhenNotLoading) {
  facade_->NotifyModelCleared();               // состояние -> Idle
  EXPECT_NO_THROW(facade_->NotifyLoadFinished(true, ""));
  // Просто не должно падать
}

// Дополнительные тесты автомата
TEST(StateMachineTest, CanCancelLoad) {
  AppStateMachine sm;
  EXPECT_FALSE(sm.CanCancelLoad());
  sm.OnLoadStarted();
  EXPECT_TRUE(sm.CanCancelLoad());
}

TEST(StateMachineTest, CanChangeSettings) {
  AppStateMachine sm;
  EXPECT_FALSE(sm.CanChangeSettings());
  sm.OnLoadStarted();
  sm.OnLoadFinished(true, "");
  EXPECT_TRUE(sm.CanChangeSettings());
}

TEST(StateMachineTest, OnError) {
  AppStateMachine sm;
  sm.OnError("test error");
  EXPECT_EQ(sm.GetState(), AppState::Error);
  EXPECT_EQ(sm.GetErrorMessage(), "test error");
}

// Покрытие RotateSelected с несколькими объектами (макрокоманда)
TEST_F(FacadeTest, RotateSelectedMultiple) {
  auto mesh2 = std::make_unique<Mesh>();
  mesh2->SetName("mesh2");
  scene_->AddObject(std::move(mesh2));
  scene_->SetSelected({scene_->GetObjects()[0].get(), scene_->GetObjects()[1].get()});
  Point oldRot0 = scene_->GetObjects()[0]->GetTransform().GetRotation();
  Point oldRot1 = scene_->GetObjects()[1]->GetTransform().GetRotation();
  facade_->RotateSelected(Point{10, 20, 30});
  auto rot0 = scene_->GetObjects()[0]->GetTransform().GetRotation();
  auto rot1 = scene_->GetObjects()[1]->GetTransform().GetRotation();
  EXPECT_FLOAT_EQ(rot0.x, oldRot0.x + 10);
  EXPECT_FLOAT_EQ(rot0.y, oldRot0.y + 20);
  EXPECT_FLOAT_EQ(rot0.z, oldRot0.z + 30);
  EXPECT_FLOAT_EQ(rot1.x, oldRot1.x + 10);
  EXPECT_FLOAT_EQ(rot1.y, oldRot1.y + 20);
  EXPECT_FLOAT_EQ(rot1.z, oldRot1.z + 30);
}

// Покрытие ScaleSelected с несколькими объектами (макрокоманда)
TEST_F(FacadeTest, ScaleSelectedMultiple) {
  auto mesh2 = std::make_unique<Mesh>();
  mesh2->SetName("mesh2");
  scene_->AddObject(std::move(mesh2));
  scene_->SetSelected({scene_->GetObjects()[0].get(), scene_->GetObjects()[1].get()});
  Point oldScale0 = scene_->GetObjects()[0]->GetTransform().GetScale();
  Point oldScale1 = scene_->GetObjects()[1]->GetTransform().GetScale();
  facade_->ScaleSelected(Point{2, 2, 2});
  auto scale0 = scene_->GetObjects()[0]->GetTransform().GetScale();
  auto scale1 = scene_->GetObjects()[1]->GetTransform().GetScale();
  EXPECT_FLOAT_EQ(scale0.x, oldScale0.x + 2);
  EXPECT_FLOAT_EQ(scale0.y, oldScale0.y + 2);
  EXPECT_FLOAT_EQ(scale0.z, oldScale0.z + 2);
  EXPECT_FLOAT_EQ(scale1.x, oldScale1.x + 2);
  EXPECT_FLOAT_EQ(scale1.y, oldScale1.y + 2);
  EXPECT_FLOAT_EQ(scale1.z, oldScale1.z + 2);
}

// Undo/Redo при заблокированном состоянии (CanTransform=false) – они просто не должны менять позицию
TEST_F(FacadeTest, UndoRedoBlocked) {
  Point oldPos = scene_->GetSelected().front()->GetTransform().GetPosition();
  facade_->MoveSelected({1, 0, 0});      // можно переместить, состояние Ready
  facade_->NotifyModelCleared();         // переходим в Idle, CanTransform = false
  facade_->Undo();                       // должен быть заблокирован
  facade_->Redo();                       // заблокирован
  auto pos = scene_->GetSelected().front()->GetTransform().GetPosition();
  EXPECT_FLOAT_EQ(pos.x, oldPos.x + 1);  // позиция не изменилась после блокировки
}
