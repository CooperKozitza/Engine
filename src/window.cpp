#include "../include/window.hpp"

#include <iostream>

eng::window::window() : glfw_window(nullptr) { 
    glfwInit(); 
    glfwSetErrorCallback([](int code, const char *description) {
      std::cerr << "A GLFW Error Occured (Error: " << code << ')' << std::endl;
      std::cerr << "- " << description << std::endl;
    });
}

eng::window::~window() {
  glfwDestroyWindow(glfw_window);
  glfwTerminate();
}

eng::window::window(const vec2<unsigned int> &res, const char *title)
    : window() {
  initialize(res, title);
}

eng::window::window(const eng::window_details &details)
    : window() {
  initialize(details.resolution, details.title);
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
}