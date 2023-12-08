#pragma once

#include <algorithm>

#include "device.hpp"
#include "instance.hpp"
#include "surface.hpp"

namespace eng {
class swap_chain {
public:
  swap_chain();
  ~swap_chain();

  /// <summary>
  /// Creates the swap chain
  /// </summary>
  void create_swap_chain(device *dev, surface *surf, window *win);

  /// <summary>
  /// Initialized each of the image views for each of the images in the swap
  /// chain
  /// </summary>
  void create_image_views(device *dev);

  VkSwapchainKHR &get() { return m_swap_chain; };

  std::vector<VkImage> &get_images() { return m_images; };
  std::vector<VkImageView> &get_image_views() {
    return m_image_views;
  };

  VkFormat &get_image_format() { return m_image_format; };
  VkExtent2D &get_extent() { return m_extent; };


private:
  /// <summary>
  /// Choose the prefered swap surface format to satisfy requirements like an
  /// sRGB color space
  /// </summary>
  /// <param name="available_formats">The list of available formats</param>
  /// <returns>The prefered format</returns>
  VkSurfaceFormatKHR choose_swap_surface_format(
      const std::vector<VkSurfaceFormatKHR> &available_formats);

  /// <summary>
  /// Chooses a present mode that is faster if available, but chooses an energy
  /// effecient mobile-friendly if not
  /// </summary>
  /// <param name="available_present_modes">The list of available modes</param>
  /// <returns>The prefered mode</returns>
  VkPresentModeKHR choose_present_mode(
      const std::vector<VkPresentModeKHR> &available_present_modes);

  /// <summary>
  /// Gets the extents of the swap chain from the frame buffer
  /// </summary>
  /// <returns>The extents</returns>
  VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities);

  GLFWwindow *m_window;
  VkSurfaceKHR m_surface;

  VkDevice m_device;
  VkPhysicalDevice m_physical_device;

  VkSwapchainKHR m_swap_chain;

  std::vector<VkImage> m_images;
  VkFormat m_image_format;

  VkExtent2D m_extent;

  std::vector<VkImageView> m_image_views;
};
} // namespace eng