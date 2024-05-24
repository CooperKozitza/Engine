#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#pragma once

namespace eng {
struct window_details {
  window_details() : resolution(), title(""), fullscreen() {}
  window_details(const glm::uvec2 &res, const char *title,
                 bool fullscreen = false)
      : resolution(res), title(title), fullscreen(fullscreen) {}

  const char *title;
  glm::uvec2 resolution;
  bool fullscreen;
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
  window(glm::uvec2 &res, const char *title, bool fullscreen = false);
  window(window_details &details);

  window(const window &) = delete;
  const window &operator=(const window &) = delete;

  void initialize_rendering(const glm::uvec2 &res, const char *title, bool fullscreen = false);

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