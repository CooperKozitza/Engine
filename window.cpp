#include "window.hpp"

eng::window::window() : glfw_window(nullptr) { glfwInit(); }

eng::window::~window() {
  glfwDestroyWindow(glfw_window);
  glfwTerminate();
}

eng::window::window(const vec2<unsigned int> &res, const char *title)
    : window() {
  initialize(res, title);
}

void eng::window::initialize(const vec2<unsigned int> &res, const char *title) {
  if (glfw_window) {
    glfwDestroyWindow(glfw_window);
  }

  // use vulkan
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  // make window not resizable
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  glfw_window = glfwCreateWindow(res.x, res.y, title, nullptr, nullptr);
  m_is_open = true;
}