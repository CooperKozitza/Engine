#include "../include/surface.hpp"

eng::surface::surface()
    : m_instance(VK_NULL_HANDLE), m_surface(VK_NULL_HANDLE) {}

eng::surface::~surface() {
  if (m_instance != VK_NULL_HANDLE && m_surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
  }
}

void eng::surface::create_surface(instance &inst, window &win) {
  m_instance = inst.get_instance();
  GLFWwindow *m_window = win.get_window();

  if (glfwCreateWindowSurface(m_instance, m_window, nullptr,
                              &m_surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }

  std::cout << "Created Window Surface" << std::endl;
}
