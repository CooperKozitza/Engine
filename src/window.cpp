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

eng::window::window(const glm::uvec2 &res, const char *title)
    : window() {
  initialize_rendering(res, title);
}

eng::window::window(const eng::window_details &details)
    : window() {
  initialize_rendering(details.resolution, details.title);
}

void eng::window::initialize_rendering(const glm::uvec2 &res, const char *title) {
  if (m_window) {
    glfwDestroyWindow(m_window);
  }

  // use vulkan
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  // make window not resizable
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  m_window = glfwCreateWindow(res.x, res.y, title, nullptr, nullptr);
}