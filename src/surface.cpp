#include "../include/surface.hpp"

eng::surface::surface()
    : vulkan_instance(VK_NULL_HANDLE), vulkan_surface(VK_NULL_HANDLE) {}

eng::surface::~surface() {
  if (vulkan_instance != VK_NULL_HANDLE && vulkan_surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, nullptr);
  }
}

void eng::surface::create_surface(instance *m_instance, window *m_window) {
  vulkan_instance = m_instance->get();
  GLFWwindow *glfw_window = m_window->get();

  if (glfwCreateWindowSurface(vulkan_instance, glfw_window, nullptr,
                              &vulkan_surface) != VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }

  std::cout << "Created Window Surface" << std::endl;
}
