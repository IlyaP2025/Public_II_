#ifndef S21_EDGE_H
#define S21_EDGE_H

#include <cstddef>

namespace s21 {

struct Edge {
  std::size_t v1, v2;

  Edge() : v1(0), v2(0) {}
  Edge(std::size_t v1, std::size_t v2) : v1(v1), v2(v2) {}
};

}  // namespace s21

#endif