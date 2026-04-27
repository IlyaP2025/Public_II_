#ifndef S21_POINT_H
#define S21_POINT_H

#include <cmath>

namespace s21 {

struct Point {
  float x, y, z;

  Point() : x(0), y(0), z(0) {}
  Point(float x, float y, float z) : x(x), y(y), z(z) {}

  Point operator+(const Point& other) const {
    return Point(x + other.x, y + other.y, z + other.z);
  }
  Point operator-(const Point& other) const {
    return Point(x - other.x, y - other.y, z - other.z);
  }
  Point& operator+=(const Point& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }
  Point& operator-=(const Point& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  float length() const { return std::sqrt(x * x + y * y + z * z); }
};

struct Point2D {
  float u, v;
  Point2D() : u(0), v(0) {}
  Point2D(float u, float v) : u(u), v(v) {}
};

}  // namespace s21

#endif  // S21_POINT_H
