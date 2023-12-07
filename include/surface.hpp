#pragma once

#include "instance.hpp"
#include "window.hpp"

namespace eng {
class surface {
public:
  surface();
  ~surface();

  /// <summary>
  /// Uses the window to create a VkSurfaceKHR
  /// </summary>
  void create_surface(instance *m_instance, window *m_window);

  VkSurfaceKHR get() { return vulkan_surface; }

private:
  VkInstance vulkan_instance;
  VkSurfaceKHR vulkan_surface;
};
}