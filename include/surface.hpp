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
  void create_surface(instance *inst, window *win);

  VkSurfaceKHR &get() { return m_surface; }

private:
  VkInstance m_instance;
  VkSurfaceKHR m_surface;
};
}