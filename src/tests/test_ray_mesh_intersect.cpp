#include <gtest/gtest.h>
#include "scene/mesh.h"
#include "scene/ray_mesh_intersect.h"
#include "tracer/ray_tracer.h"   // для HitRecord

using namespace s21;

TEST(RayMeshIntersectTest, HitTriangle) {
  Mesh mesh;
  mesh.SetVertices({{0,0,0}, {1,0,0}, {0,1,0}});
  mesh.SetTriangles({0,1,2});
  Ray ray{Point(0.25, 0.25, -1), Point(0,0,1)};  // луч идёт вверх, попадает в треугольник
  HitRecord rec;
  bool hit = RayMeshIntersect(mesh, ray, 0.0f, 100.0f, rec);
  ASSERT_TRUE(hit);
  EXPECT_NEAR(rec.t, 1.0, 0.01);
  EXPECT_NEAR(rec.point.x, 0.25, 0.01);
  EXPECT_NEAR(rec.point.y, 0.25, 0.01);
  EXPECT_NEAR(rec.point.z, 0, 0.01);
  EXPECT_NEAR(rec.normal.x, 0, 0.01);
  EXPECT_NEAR(rec.normal.y, 0, 0.01);
  EXPECT_NEAR(rec.normal.z, 1, 0.01);
}

TEST(RayMeshIntersectTest, MissTriangle) {
  Mesh mesh;
  mesh.SetVertices({{0,0,0}, {1,0,0}, {0,1,0}});
  mesh.SetTriangles({0,1,2});
  Ray ray{Point(0.1, 0.1, -1), Point(1,0,0)};
  HitRecord rec;
  bool hit = RayMeshIntersect(mesh, ray, 0.0f, 100.0f, rec);
  EXPECT_FALSE(hit);
}

TEST(RayMeshIntersectTest, EmptyMesh) {
  Mesh mesh;
  Ray ray{Point(0,0,-1), Point(0,0,1)};
  HitRecord rec;
  EXPECT_FALSE(RayMeshIntersect(mesh, ray, 0.0f, 100.0f, rec));
}

TEST(RayMeshIntersectTest, CubeIntersection) {
  Mesh mesh;
  mesh.SetVertices({
    {0,0,0},{1,0,0},{1,1,0},{0,1,0},
    {0,0,1},{1,0,1},{1,1,1},{0,1,1}
  });
  mesh.SetTriangles({
    0,1,2, 0,2,3, 4,5,6, 4,6,7,
    0,1,5, 0,5,4, 2,3,7, 2,7,6,
    0,3,7, 0,7,4, 1,2,6, 1,6,5
  });
  Ray ray{Point(0.5, 0.5, -2), Point(0,0,1)};
  HitRecord rec;
  bool hit = RayMeshIntersect(mesh, ray, 0.0f, 100.0f, rec);
  ASSERT_TRUE(hit);
  EXPECT_NEAR(rec.t, 2.0, 0.01);
}
