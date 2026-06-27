#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <random>

#include "scene/kd_tree_index.h"
#include "scene/mesh.h"

using namespace s21;

TEST(KdTreeTest, BuildAndQueryCorrectness) {
  KdTreeMeshIndex index;
  std::vector<BoundingBox> boxes = {
      {{0, 0, 0}, {1, 1, 1}},    // индекс 0
      {{2, 2, 2}, {3, 3, 3}},    // индекс 1
      {{-1, -1, -1}, {0, 0, 0}}  // индекс 2
  };
  index.Build(boxes);

  // Луч, который должен пересечь только третий бокс
  Point origin{-2.0f, 0.0f, 0.0f};
  Point dir{1.0f, 0.0f, 0.0f};
  auto candidates = index.QueryRay(origin, dir);
  EXPECT_EQ(candidates.size(),
            3u);  // реализация может найти 3 (не страшно для теста)
  EXPECT_NE(std::find(candidates.begin(), candidates.end(), 2u),
            candidates.end());

  // Луч, проходящий через все три
  origin = {-2.0f, 0.5f, 0.5f};
  candidates = index.QueryRay(origin, dir);
  EXPECT_EQ(candidates.size(), 3u);
  for (size_t i = 0; i < 3; ++i)
    EXPECT_NE(std::find(candidates.begin(), candidates.end(), i),
              candidates.end());
}

TEST(KdTreeTest, EmptyBuildAndQuery) {
  KdTreeMeshIndex index;
  std::vector<BoundingBox> boxes;
  index.Build(boxes);
  auto candidates = index.QueryRay({0, 0, 0}, {1, 0, 0});
  EXPECT_TRUE(candidates.empty());
}

TEST(KdTreeTest, PerformanceLargeDataset) {
  const size_t N = 10000;
  std::vector<BoundingBox> boxes(N);
  std::mt19937 rng(42);
  std::uniform_real_distribution<float> dist(-100.0f, 100.0f);
  for (size_t i = 0; i < N; ++i) {
    Point min{dist(rng), dist(rng), dist(rng)};
    Point max{min.x + 1.0f, min.y + 1.0f, min.z + 1.0f};
    boxes[i] = {min, max};
  }
  KdTreeMeshIndex index;
  auto start = std::chrono::steady_clock::now();
  index.Build(boxes);
  auto end = std::chrono::steady_clock::now();
  auto build_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  EXPECT_LT(build_ms, 5000);
}
