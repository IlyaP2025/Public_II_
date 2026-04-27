#include <gtest/gtest.h>

#include "scene/mesh.h"
#include "scene/scene.h"
#include "scene/scene_observer.h"

using namespace s21;

// Mock observer для проверки вызовов
class MockSceneObserver : public SceneObserver {
 public:
  void OnObjectAdded(SceneObject* obj) override { added = obj; }
  void OnObjectRemoved(SceneObject* obj) override { removed = obj; }
  void OnSelectionChanged(const std::vector<SceneObject*>& sel) override {
    selected = sel;
  }
  void OnTransformChanged(SceneObject* obj) override { transformed = obj; }

  SceneObject* added = nullptr;
  SceneObject* removed = nullptr;
  std::vector<SceneObject*> selected;
  SceneObject* transformed = nullptr;
};

TEST(SceneTest, AddObject) {
  Scene scene;
  auto mesh = std::make_unique<Mesh>();
  Mesh* ptr = mesh.get();
  scene.AddObject(std::move(mesh));
  const auto& objects = scene.GetObjects();
  ASSERT_EQ(objects.size(), 1);
  EXPECT_EQ(objects[0].get(), ptr);
}

TEST(SceneTest, AddObjectNotifiesObserver) {
  Scene scene;
  MockSceneObserver obs;
  scene.AddObserver(&obs);
  auto mesh = std::make_unique<Mesh>();
  Mesh* ptr = mesh.get();
  scene.AddObject(std::move(mesh));
  EXPECT_EQ(obs.added, ptr);
  scene.RemoveObserver(&obs);
}

TEST(SceneTest, RemoveObject) {
  Scene scene;
  auto mesh = std::make_unique<Mesh>();
  Mesh* ptr = mesh.get();
  scene.AddObject(std::move(mesh));
  scene.RemoveObject(ptr);
  EXPECT_TRUE(scene.GetObjects().empty());
}

TEST(SceneTest, RemoveObjectNotifiesObserver) {
  Scene scene;
  MockSceneObserver obs;
  scene.AddObserver(&obs);
  auto mesh = std::make_unique<Mesh>();
  Mesh* ptr = mesh.get();
  scene.AddObject(std::move(mesh));
  scene.RemoveObject(ptr);
  EXPECT_EQ(obs.removed, ptr);
  scene.RemoveObserver(&obs);
}

TEST(SceneTest, SetSelected) {
  Scene scene;
  auto mesh = std::make_unique<Mesh>();
  Mesh* ptr = mesh.get();
  scene.AddObject(std::move(mesh));
  scene.SetSelected({ptr});
  const auto& sel = scene.GetSelected();
  ASSERT_EQ(sel.size(), 1);
  EXPECT_EQ(sel[0], ptr);
}

TEST(SceneTest, SetSelectedNotifiesObserver) {
  Scene scene;
  MockSceneObserver obs;
  scene.AddObserver(&obs);
  auto mesh = std::make_unique<Mesh>();
  Mesh* ptr = mesh.get();
  scene.AddObject(std::move(mesh));
  scene.SetSelected({ptr});
  ASSERT_EQ(obs.selected.size(), 1);
  EXPECT_EQ(obs.selected[0], ptr);
  scene.RemoveObserver(&obs);
}

TEST(SceneTest, NotifyTransformChanged) {
  Scene scene;
  MockSceneObserver obs;
  scene.AddObserver(&obs);
  auto mesh = std::make_unique<Mesh>();
  Mesh* ptr = mesh.get();
  scene.AddObject(std::move(mesh));
  scene.NotifyTransformChanged(ptr);
  EXPECT_EQ(obs.transformed, ptr);
  scene.RemoveObserver(&obs);
}

TEST(SceneTest, GetAllMeshes) {
  Scene scene;
  auto mesh1 = std::make_unique<Mesh>();
  auto mesh2 = std::make_unique<Mesh>();
  // Добавим ещё не-Mesh объект (если есть другой тип)
  struct NonMesh : public SceneObject {};
  auto non = std::make_unique<NonMesh>();

  Mesh* ptr1 = mesh1.get();
  Mesh* ptr2 = mesh2.get();
  scene.AddObject(std::move(mesh1));
  scene.AddObject(std::move(mesh2));
  scene.AddObject(std::move(non));

  auto meshes = scene.GetAllMeshes();
  EXPECT_EQ(meshes.size(), 2);
  EXPECT_TRUE(meshes[0] == ptr1 || meshes[0] == ptr2);
  EXPECT_TRUE(meshes[1] == ptr1 || meshes[1] == ptr2);
}

TEST(SceneTest, Clear) {
  Scene scene;
  auto mesh = std::make_unique<Mesh>();
  scene.AddObject(std::move(mesh));
  scene.Clear();
  EXPECT_TRUE(scene.GetObjects().empty());
  // Также проверяем, что выделение сброшено
  EXPECT_TRUE(scene.GetSelected().empty());
}

TEST(SceneTest, ClearRemovesAllObjectsAndResetsSelection) {
  Scene scene;
  auto mesh1 = std::make_unique<Mesh>();
  auto mesh2 = std::make_unique<Mesh>();
  Mesh* ptr1 = mesh1.get();
  Mesh* ptr2 = mesh2.get();
  scene.AddObject(std::move(mesh1));
  scene.AddObject(std::move(mesh2));
  scene.SetSelected({ptr1, ptr2});

  EXPECT_EQ(scene.GetObjects().size(), 2u);
  EXPECT_EQ(scene.GetSelected().size(), 2u);

  scene.Clear();

  EXPECT_TRUE(scene.GetObjects().empty());
  EXPECT_TRUE(scene.GetSelected().empty());
  // Указатели ptr1 и ptr2 теперь невалидны, но это ожидаемо.
}
