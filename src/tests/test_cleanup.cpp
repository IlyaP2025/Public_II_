#include <gtest/gtest.h>

#include <cmath>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "loaders/obj_loader.h"
#include "scene/mesh.h"
#include "scene/model_builder.h"
#include "scene/scene.h"
#include "settings/settings.h"

using namespace s21;

class SceneCleanupTest : public ::testing::Test {
 protected:
  void SetUp() override {
    cube_content_ = R"(
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
v 0 0 1
v 1 0 1
v 1 1 1
v 0 1 1
f 1 2 3 4
f 5 6 7 8
f 1 2 6 5
f 2 3 7 6
f 3 4 8 7
f 4 1 5 8
)";
    elephant_content_ = R"(
v 0 0 0
v 1 0 0
v 0 1 0
f 1 2 3
)";
  }

  std::string CreateTempFile(const std::string& content) {
    char filename[] = "/tmp/test_cleanup_XXXXXX";
    int fd = mkstemp(filename);
    if (fd == -1) throw std::runtime_error("Cannot create temp file");
    close(fd);
    std::ofstream file(filename);
    file << content;
    file.close();
    return std::string(filename);
  }

  void RemoveTempFile(const std::string& filename) {
    std::remove(filename.c_str());
  }

  std::string cube_content_;
  std::string elephant_content_;
};

TEST_F(SceneCleanupTest, LoadMultipleModelsAndCheckNoLeaks) {
  Scene scene;
  ObjLoader loader;

  std::string cube_file = CreateTempFile(cube_content_);
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;
  bool ok = loader.Load(cube_file, objects, error);
  RemoveTempFile(cube_file);
  ASSERT_TRUE(ok) << error;
  ASSERT_EQ(objects.size(), 1u);
  Mesh* cube_mesh = dynamic_cast<Mesh*>(objects[0].get());
  ASSERT_NE(cube_mesh, nullptr);
  scene.AddObject(std::move(objects[0]));

  EXPECT_EQ(scene.GetAllMeshes().size(), 1u);
  EXPECT_EQ(cube_mesh->VertexCount(), 36u);
  EXPECT_EQ(cube_mesh->EdgeCount(), 36u);

  scene.Clear();
  EXPECT_EQ(scene.GetAllMeshes().size(), 0u);
  // cube_mesh теперь висячий, не используем его

  objects.clear();

  std::string elephant_file = CreateTempFile(elephant_content_);
  ok = loader.Load(elephant_file, objects, error);
  RemoveTempFile(elephant_file);
  ASSERT_TRUE(ok);
  ASSERT_EQ(objects.size(), 1u);
  Mesh* elephant_mesh = dynamic_cast<Mesh*>(objects[0].get());
  ASSERT_NE(elephant_mesh, nullptr);
  scene.AddObject(std::move(objects[0]));

  EXPECT_EQ(scene.GetAllMeshes().size(), 1u);
  EXPECT_EQ(elephant_mesh->VertexCount(), 3u);
  EXPECT_EQ(elephant_mesh->EdgeCount(), 3u);
}

TEST_F(SceneCleanupTest, SmoothingFactorPreservedAcrossLoads) {
  Settings::instance().setSmoothingFactor(0.3f);
  ObjLoader loader;
  std::string cube_file = CreateTempFile(cube_content_);
  RawModelData data =
      ObjLoader::LoadData(cube_file, Settings::instance().smoothingFactor());
  RemoveTempFile(cube_file);
  ASSERT_TRUE(data.success);
  EXPECT_FLOAT_EQ(data.smoothingFactor, 0.3f);

  auto mesh = ModelBuilder::BuildFromRawData(data);
  const auto& normals = mesh->GetNormals();
  ASSERT_FALSE(normals.empty());

  Mesh temp_mesh;
  temp_mesh.SetVertices(data.vertices);
  temp_mesh.SetTriangles(data.triangles);
  temp_mesh.SetFlatNormals(data.flat_normals);
  temp_mesh.SetSmoothNormals(data.smooth_normals);
  temp_mesh.ComputeNormals(0.0f);
  auto normals_flat = temp_mesh.GetNormals();
  temp_mesh.ComputeNormals(1.0f);
  auto normals_smooth = temp_mesh.GetNormals();

  bool differs_from_flat = false;
  bool differs_from_smooth = false;
  for (size_t i = 0; i < normals.size(); ++i) {
    if (std::abs(normals[i].x - normals_flat[i].x) > 1e-4f)
      differs_from_flat = true;
    if (std::abs(normals[i].x - normals_smooth[i].x) > 1e-4f)
      differs_from_smooth = true;
  }
  EXPECT_TRUE(differs_from_flat);
  EXPECT_TRUE(differs_from_smooth);
}
