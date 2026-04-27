#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "common/edge.h"
#include "common/point.h"
#include "loaders/loader_factory.h"
#include "loaders/obj_loader.h"
#include "scene/mesh.h"
#include "scene/model_builder.h"  // <-- добавлено
#include "scene/scene_object.h"
// #include "settings/settings.h"  // <-- добавлено на всякий случай

namespace s21 {
namespace {

// Вспомогательная функция для создания временного файла с содержимым
std::string CreateTempObjFile(const std::string& content) {
  char filename[] = "/tmp/test_obj_XXXXXX";  // шаблон для mkstemp
  int fd = mkstemp(filename);
  if (fd == -1) {
    throw std::runtime_error("Cannot create temporary file");
  }
  close(fd);  // закрываем дескриптор, далее работаем через потоки

  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Cannot open temporary file for writing");
  }
  file << content;
  file.close();
  return std::string(filename);
}

// Удаление временного файла
void RemoveTempFile(const std::string& filename) {
  std::remove(filename.c_str());
}

// Тестовый fixture для управления временными файлами
class ObjLoaderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Пусто, файлы создаются в каждом тесте
  }

  void TearDown() override {
    for (const auto& f : temp_files_) {
      RemoveTempFile(f);
    }
    temp_files_.clear();
  }

  std::string CreateFile(const std::string& content) {
    std::string filename = CreateTempObjFile(content);
    temp_files_.push_back(filename);
    return filename;
  }

 private:
  std::vector<std::string> temp_files_;
};

TEST_F(ObjLoaderTest, LoadCube) {
  // .obj куба: 8 вершин, 6 квадратных граней -> 12 рёбер
  std::string content = R"(
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
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_TRUE(success) << "Error: " << error;
  EXPECT_TRUE(error.empty());
  ASSERT_EQ(objects.size(), 1);

  // Проверяем, что объект — Mesh
  auto* mesh = dynamic_cast<Mesh*>(objects[0].get());
  ASSERT_NE(mesh, nullptr);

  // Вершины
  EXPECT_EQ(mesh->GetVertices().size(), 36);
  // Рёбра: у куба 12 рёбер
  EXPECT_EQ(mesh->GetEdges().size(), 36);
}

TEST_F(ObjLoaderTest, LoadTetrahedron) {
  // Тетраэдр: 4 вершины, 4 треугольные грани -> 6 рёбер
  std::string content = R"(
v 0 0 0
v 1 0 0
v 0 1 0
v 0 0 1
f 1 2 3
f 1 2 4
f 1 3 4
f 2 3 4
)";
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_TRUE(success);
  ASSERT_EQ(objects.size(), 1);

  auto* mesh = dynamic_cast<Mesh*>(objects[0].get());
  ASSERT_NE(mesh, nullptr);
  EXPECT_EQ(mesh->GetVertices().size(), 12);
  EXPECT_EQ(mesh->GetEdges().size(), 12);
}

TEST_F(ObjLoaderTest, NegativeIndices) {
  // Используем отрицательные индексы (отсчёт от конца)
  std::string content = R"(
v 0 0 0
v 1 0 0
v 1 1 0
v 0 1 0
f 1 -3 -2
)";
  // Ожидаем грань: 1, (4-3+1)=2? На самом деле -3 при 4 вершинах даёт индекс 2
  // (0-based: 1) Разберём: -1 -> последняя (4), -2 -> предпоследняя (3), -3 ->
  // (2) Индексы в файле: 1, -3, -2 => преобразуются: 0, 2, 3 (0-based) Это
  // треугольник из вершин 1,3,4 (v1,v3,v4)
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_TRUE(success);
  ASSERT_EQ(objects.size(), 1);

  auto* mesh = dynamic_cast<Mesh*>(objects[0].get());
  ASSERT_NE(mesh, nullptr);
  EXPECT_EQ(mesh->GetVertices().size(), 3);
  // Для треугольника 3 ребра
  EXPECT_EQ(mesh->GetEdges().size(), 3);
}

TEST_F(ObjLoaderTest, IndicesWithTextureAndNormal) {
  // Формат f v/vt/vn
  std::string content = R"(
v 0 0 0
v 1 0 0
v 0 1 0
f 1/1/1 2/2/2 3/3/3
)";
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_TRUE(success);
  ASSERT_EQ(objects.size(), 1);

  auto* mesh = dynamic_cast<Mesh*>(objects[0].get());
  ASSERT_NE(mesh, nullptr);
  EXPECT_EQ(mesh->GetVertices().size(), 3);
  EXPECT_EQ(mesh->GetEdges().size(), 3);
}

TEST_F(ObjLoaderTest, IgnoreOtherLines) {
  // Проверяем, что строки vt, vn, g, o игнорируются
  std::string content = R"(
v 0 0 0
v 1 0 0
v 0 1 0
vt 0 0
vn 0 0 1
g MyGroup
o ObjectName
f 1 2 3
)";
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_TRUE(success);
  ASSERT_EQ(objects.size(), 1);

  auto* mesh = dynamic_cast<Mesh*>(objects[0].get());
  ASSERT_NE(mesh, nullptr);
  EXPECT_EQ(mesh->GetVertices().size(), 3);
  EXPECT_EQ(mesh->GetEdges().size(), 3);
}

TEST_F(ObjLoaderTest, NoVertices) {
  std::string content = "f 1 2 3\n";
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_FALSE(success);
  EXPECT_FALSE(error.empty());
  EXPECT_TRUE(objects.empty());
}

TEST_F(ObjLoaderTest, NoFaces) {
  std::string content = "v 0 0 0\nv 1 0 0\n";
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_FALSE(success);
  EXPECT_FALSE(error.empty());
  EXPECT_TRUE(objects.empty());
}

TEST_F(ObjLoaderTest, InvalidVertexLine) {
  std::string content = "v 0 0\n";  // не хватает z
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_FALSE(success);
  EXPECT_FALSE(error.empty());
}

TEST_F(ObjLoaderTest, FaceWithTwoVertices) {
  std::string content =
      "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2\n";  // грань из 2 вершин
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_FALSE(success);
  EXPECT_FALSE(error.empty());
}

TEST_F(ObjLoaderTest, IndexOutOfRange) {
  std::string content =
      "v 0 0 0\nv 1 0 0\nf 1 2 3\n";  // индекс 3 не существует
  std::string filename = CreateFile(content);

  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load(filename, objects, error);
  EXPECT_FALSE(success);
  EXPECT_FALSE(error.empty());
}

TEST_F(ObjLoaderTest, FileNotFound) {
  ObjLoader loader;
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;

  bool success = loader.Load("nonexistent.obj", objects, error);
  EXPECT_FALSE(success);
  EXPECT_FALSE(error.empty());
}

// Тест фабрики загрузчиков
TEST(LoaderFactoryTest, RegisterAndCreate) {
  auto& factory = LoaderFactory::Instance();

  // Регистрируем загрузчик для .obj
  factory.Register("obj", []() { return std::make_unique<ObjLoader>(); });

  auto loader = factory.CreateLoader("obj");
  EXPECT_NE(loader, nullptr);

  auto unknown = factory.CreateLoader("txt");
  EXPECT_EQ(unknown, nullptr);
}

TEST_F(ObjLoaderTest, LoadDataValidFile) {
  std::string content = R"(
v 0 0 0
v 1 0 0
v 0 1 0
f 1 2 3
)";
  std::string filename = CreateFile(content);
  RawModelData data = ObjLoader::LoadData(filename);
  EXPECT_TRUE(data.success);
  EXPECT_EQ(data.vertices.size(), 3);
  EXPECT_EQ(data.edges.size(), 3);
  EXPECT_EQ(data.filename, filename);
  EXPECT_TRUE(data.errorMsg.empty());
}

TEST_F(ObjLoaderTest, LoadDataInvalidFile) {
  RawModelData data = ObjLoader::LoadData("nonexistent.obj");
  EXPECT_FALSE(data.success);
  EXPECT_FALSE(data.errorMsg.empty());
}

TEST_F(ObjLoaderTest, LoadDataNoVertices) {
  std::string content = "f 1 2 3\n";
  std::string filename = CreateFile(content);
  RawModelData data = ObjLoader::LoadData(filename);
  EXPECT_FALSE(data.success);
  EXPECT_FALSE(data.errorMsg.empty());
}

TEST_F(ObjLoaderTest, LoadDataNoFaces) {
  std::string content = "v 0 0 0\nv 1 0 0\n";
  std::string filename = CreateFile(content);
  RawModelData data = ObjLoader::LoadData(filename);
  EXPECT_FALSE(data.success);
  EXPECT_FALSE(data.errorMsg.empty());
}

TEST_F(ObjLoaderTest, LoadDataInvalidFace) {
  std::string content =
      "v 0 0 0\nv 1 0 0\nf 1 2 3\n";  // индекс 3 не существует
  std::string filename = CreateFile(content);
  RawModelData data = ObjLoader::LoadData(filename);
  EXPECT_FALSE(data.success);
  EXPECT_FALSE(data.errorMsg.empty());
}

TEST_F(ObjLoaderTest, SmoothingFactorChangesNormals) {
  std::string content = R"(
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
  std::string filename = CreateFile(content);

  // Загружаем с factor = 0 (плоские)
  RawModelData data_flat = ObjLoader::LoadData(filename, 0.0f);
  ASSERT_TRUE(data_flat.success);
  auto mesh_flat = ModelBuilder::BuildFromRawData(data_flat);
  const auto& normals_flat = mesh_flat->GetNormals();

  // Загружаем с factor = 1 (гладкие)
  RawModelData data_smooth = ObjLoader::LoadData(filename, 1.0f);
  ASSERT_TRUE(data_smooth.success);
  auto mesh_smooth = ModelBuilder::BuildFromRawData(data_smooth);
  const auto& normals_smooth = mesh_smooth->GetNormals();

  ASSERT_EQ(normals_flat.size(), normals_smooth.size());
  bool any_different = false;
  for (size_t i = 0; i < normals_flat.size(); ++i) {
    if (std::abs(normals_flat[i].x - normals_smooth[i].x) > 1e-4f ||
        std::abs(normals_flat[i].y - normals_smooth[i].y) > 1e-4f ||
        std::abs(normals_flat[i].z - normals_smooth[i].z) > 1e-4f) {
      any_different = true;
      break;
    }
  }
  EXPECT_TRUE(any_different);
}

TEST_F(ObjLoaderTest, RawModelDataContainsFlatAndSmoothNormals) {
  std::string content = R"(
v 0 0 0
v 1 0 0
v 0 1 0
f 1 2 3
)";
  std::string filename = CreateFile(content);
  RawModelData data = ObjLoader::LoadData(filename, 0.5f);
  ASSERT_TRUE(data.success);
  EXPECT_EQ(data.flat_normals.size(), data.vertices.size());
  EXPECT_EQ(data.smooth_normals.size(), data.vertices.size());
  // Плоские и гладкие нормали должны быть разными для куба (у треугольника
  // одна грань, но вершины могут быть разными)
  // Для треугольника с 3 вершинами, каждая вершина принадлежит только одному
  // треугольнику, поэтому плоские и гладкие нормали совпадут. Это нормально.
}

TEST_F(ObjLoaderTest, GenerateUVWhenMissing) {
  std::string content = R"(
v 0 0 0
v 1 0 0
v 0 1 0
f 1 2 3
)";
  std::string filename = CreateFile(content);
  RawModelData data = ObjLoader::LoadData(filename);
  ASSERT_TRUE(data.success);
  // UV должны быть сгенерированы
  EXPECT_FALSE(data.uvs.empty());
  EXPECT_EQ(data.uvs.size(), data.vertices.size());
  // Проверим через ModelBuilder
  auto mesh = ModelBuilder::BuildFromRawData(data);
  EXPECT_FALSE(mesh->GetUVs().empty());
}

}  // namespace
}  // namespace s21
