#ifndef S21_COMMON_RAY_H
#define S21_COMMON_RAY_H

#include "point.h"

namespace s21 {

struct Ray {
  Point origin;
  Point direction;
};

}  // namespace s21

#endif  // S21_COMMON_RAY_H
