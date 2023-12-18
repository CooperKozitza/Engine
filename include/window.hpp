#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#pragma once

namespace eng {
struct window_details {
  window_details(const glm::uvec2 &res, const char *title)
      : resolution(res), title(title){};

  const char *title;
  const glm::uvec2 resolution;
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
  window(const glm::uvec2 &res, const char *title);
  window(const window_details &details);

  window(const window &) = delete;
  const window &operator=(const window &) = delete;

  void initialize(const glm::uvec2 &res, const char *title);

  /// <summary>
  /// Checks for events like key-strokes or pressing the close button
  /// </summary>
  void poll_events() { glfwPollEvents(); }
  bool should_close() { return glfwWindowShouldClose(m_window); }

  GLFWwindow *get_window() { return m_window; }

private:
  GLFWwindow *m_window;
};
} // namespace eng