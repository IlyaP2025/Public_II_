#include <gtest/gtest.h>

#include "scene/mesh.h"

using namespace s21;

TEST(MeshTest, DefaultConstruction) {
  Mesh mesh;
  EXPECT_EQ(mesh.VertexCount(), 0);
  EXPECT_EQ(mesh.EdgeCount(), 0);
}

TEST(MeshTest, AddVertex) {
  Mesh mesh;
  mesh.AddVertex(Point(1, 2, 3));
  EXPECT_EQ(mesh.VertexCount(), 1);
  EXPECT_EQ(mesh.GetVertices()[0].x, 1);
  EXPECT_EQ(mesh.GetVertices()[0].y, 2);
  EXPECT_EQ(mesh.GetVertices()[0].z, 3);
}

TEST(MeshTest, AddEdge) {
  Mesh mesh;
  mesh.AddEdge(Edge(0, 1));
  EXPECT_EQ(mesh.EdgeCount(), 1);
  EXPECT_EQ(mesh.GetEdges()[0].v1, 0);
  EXPECT_EQ(mesh.GetEdges()[0].v2, 1);
}

TEST(MeshTest, SetVertices) {
  Mesh mesh;
  std::vector<Point> verts = {Point(1, 2, 3), Point(4, 5, 6)};
  mesh.SetVertices(verts);
  EXPECT_EQ(mesh.VertexCount(), 2);
  EXPECT_EQ(mesh.GetVertices()[1].x, 4);
}

TEST(MeshTest, Clear) {
  Mesh mesh;
  mesh.AddVertex(Point(1, 2, 3));
  mesh.AddEdge(Edge(0, 1));
  mesh.Clear();
  EXPECT_EQ(mesh.VertexCount(), 0);
  EXPECT_EQ(mesh.EdgeCount(), 0);
}

// Дополнительные тесты для методов SceneObject в Mesh
TEST(MeshTest, Name) {
  Mesh mesh;
  mesh.SetName("MyMesh");
  EXPECT_EQ(mesh.GetName(), "MyMesh");
}

TEST(MeshTest, SourceFile) {
  Mesh mesh;
  mesh.SetSourceFile("test.obj");
  EXPECT_EQ(mesh.GetSourceFile(), "test.obj");
}

TEST(MeshTest, Visibility) {
  Mesh mesh;
  EXPECT_TRUE(mesh.IsVisible());
  mesh.SetVisible(false);
  EXPECT_FALSE(mesh.IsVisible());
}

TEST(MeshTest, Selected) {
  Mesh mesh;
  EXPECT_FALSE(mesh.IsSelected());
  mesh.SetSelected(true);
  EXPECT_TRUE(mesh.IsSelected());
}

TEST(MeshTest, Transform) {
  Mesh mesh;
  Transform t;
  t.SetPosition(Point(1, 2, 3));
  mesh.SetTransform(t);
  EXPECT_EQ(mesh.GetTransform().GetPosition().x, 1);
}

TEST(MeshTest, GetBoundingBox) {
  Mesh mesh;
  // Сначала пустой
  BoundingBox box = mesh.GetBoundingBox();
  EXPECT_EQ(box.min.x, 0);
  EXPECT_EQ(box.min.y, 0);
  EXPECT_EQ(box.min.z, 0);
  EXPECT_EQ(box.max.x, 0);
  EXPECT_EQ(box.max.y, 0);
  EXPECT_EQ(box.max.z, 0);

  mesh.AddVertex(Point(1, 2, 3));
  mesh.AddVertex(Point(4, 5, 6));
  box = mesh.GetBoundingBox();
  EXPECT_EQ(box.min.x, 1);
  EXPECT_EQ(box.min.y, 2);
  EXPECT_EQ(box.min.z, 3);
  EXPECT_EQ(box.max.x, 4);
  EXPECT_EQ(box.max.y, 5);
  EXPECT_EQ(box.max.z, 6);
}

TEST(MeshTest, ComputeBoundingSphereEmpty) {
  Mesh mesh;
  mesh.ComputeBoundingSphere();
  const BoundingSphere& sphere = mesh.GetBoundingSphere();
  EXPECT_EQ(sphere.center.x, 0);
  EXPECT_EQ(sphere.center.y, 0);
  EXPECT_EQ(sphere.center.z, 0);
  EXPECT_EQ(sphere.radius, 0);
}

TEST(MeshTest, FlatAndSmoothNormals) {
  Mesh mesh;
  std::vector<Point> flat = {Point(1, 0, 0)};
  std::vector<Point> smooth = {Point(0, 1, 0)};
  mesh.SetFlatNormals(flat);
  mesh.SetSmoothNormals(smooth);
  EXPECT_EQ(mesh.GetFlatNormals().size(), 1u);
  EXPECT_EQ(mesh.GetSmoothNormals().size(), 1u);
}

TEST(MeshTest, TrianglesAccess) {
  Mesh mesh;
  std::vector<unsigned int> tris = {0, 1, 2, 3, 4, 5};
  mesh.SetTriangles(tris);
  EXPECT_EQ(mesh.GetTriangles(), tris);
  EXPECT_EQ(mesh.TriangleCount(), 2u);
}

TEST(MeshTest, ComputeNormalsWithMissingData) {
  Mesh mesh;
  mesh.SetVertices({Point(0, 0, 0), Point(1, 0, 0), Point(0, 1, 0)});
  mesh.SetTriangles({0, 1, 2});
  // Не заданы flat/smooth нормали
  mesh.ComputeNormals(0.5f);
  // Должен сработать ранний возврат, normals_ останется пустым
  EXPECT_TRUE(mesh.GetNormals().empty());
}

TEST(MeshTest, OptimizeTriangulationCoverage) {
  // Создаём куб (уже загружали в других тестах, но для покрытия вызовем явно)
  Mesh mesh;
  // Вершины куба
  std::vector<Point> verts = {{0, 0, 0}, {1, 0, 0}, {1, 1, 0}, {0, 1, 0},
                              {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}};
  // Треугольники (12)
  std::vector<unsigned int> tris = {
      0, 1, 2, 0, 2, 3,  // front
      4, 5, 6, 4, 6, 7,  // back
      0, 1, 5, 0, 5, 4,  // bottom
      2, 3, 7, 2, 7, 6,  // top
      0, 3, 7, 0, 7, 4,  // left
      1, 2, 6, 1, 6, 5   // right
  };
  mesh.SetVertices(verts);
  mesh.SetTriangles(tris);
  mesh.OptimizeTriangulation(3);
  // После оптимизации количество треугольников не изменится (они уже
  // оптимальны)
  EXPECT_EQ(mesh.TriangleCount(), 12u);
}
