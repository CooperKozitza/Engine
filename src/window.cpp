#include "../include/window.hpp"

#include <iostream>

eng::window::window() : m_window(nullptr) {
  glfwInit();
  glfwSetErrorCallback([](int code, const char *description) {
    std::cerr << "A GLFW Error Occured (Error: " << code << ')' << std::endl;
    std::cerr << "- " << description << std::endl;
  });
}

eng::window::~window() {
  glfwDestroyWindow(m_window);
  glfwTerminate();
}

eng::window::window(glm::uvec2 &res, const char *title, bool fullscreen)
    : window() {
  if (fullscreen) {
    GLFWmonitor *fullscreen_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(fullscreen_monitor);

    res.x = mode->width;
    res.y = mode->height;
  }

  initialize_rendering(res, title, fullscreen);
}

eng::window::window(eng::window_details &details) : window() {
  if (details.fullscreen) {
    GLFWmonitor *fullscreen_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode *mode = glfwGetVideoMode(fullscreen_monitor);

    details.resolution.x = mode->width;
    details.resolution.y = mode->height;
  }

  initialize_rendering(details.resolution, details.title, details.fullscreen);
}

void eng::window::initialize_rendering(const glm::uvec2 &res, const char *title,
                                       bool fullscreen) {
  if (m_window) {
    glfwDestroyWindow(m_window);
  }

  // use vulkan
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // make window not resizable
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  GLFWmonitor *fullscreen_monitor = fullscreen ? glfwGetPrimaryMonitor() : nullptr;
  m_window = glfwCreateWindow(res.x, res.y, title, fullscreen_monitor, nullptr);
}