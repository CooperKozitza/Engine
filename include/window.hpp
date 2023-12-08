#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#pragma once

#include "vectors.hpp"

namespace eng {
struct window_details {
  window_details(const vec2<uint32_t> &res, const char *title)
      : resolution(res), title(title){};

  const char *title;
  const vec2<uint32_t> resolution;
};

class window {
public:
  window();
  ~window();

  /// <summary>
  /// Creates a new window with specified height, width, and name
  /// </summary>
  /// <param name="res">The size of the window</param>
  /// <param name="title">The title of the window</param>
  window(const vec2<uint32_t> &res, const char *title);
  window(const window_details &details);

  window(const window &) = delete;
  const window &operator=(const window &) = delete;

  void initialize(const vec2<uint32_t> &res, const char *title);

  /// <summary>
  /// Checks for events like key-strokes or pressing the close button
  /// </summary>
  void poll_events() { glfwPollEvents(); }
  bool should_close() { return glfwWindowShouldClose(m_window); }

  GLFWwindow *get() { return m_window; }

private:
  GLFWwindow *m_window;
};
} // namespace eng