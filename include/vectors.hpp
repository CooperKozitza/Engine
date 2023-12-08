#pragma once

namespace eng {
template <typename T> struct vec2 {
  T x, y;
};

template <typename T> struct vec3 {
  T x, y, z;
};

struct color {
  float r, g, b, a;
};
} // namespace eng