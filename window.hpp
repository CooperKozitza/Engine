#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vectors.hpp"

namespace eng {
class window {
public:
  window();
  ~window();

  /// <summary>
  /// Creates a new window with specified height, width, and name
  /// </summary>
  /// <param name="res">The size of the window</param>
  /// <param name="title">The title of the window</param>
  window(const vec2<unsigned int> &res, const char *title);


  window(const window &) = delete;
  const window &operator=(const window &) = delete;

  void initialize(const vec2<unsigned int> &res, const char *title);

  /// <summary>
  /// Checks for events like key-strokes or pressing the close button
  /// </summary>
  void poll_events() { glfwPollEvents(); }
  bool should_close() { return glfwWindowShouldClose(glfw_window); }

private:
  bool m_is_open;
  GLFWwindow *glfw_window;
};
} // namespace eng