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
  void create_swap_chain(device *dev, surface *surf);

  /// <summary>
  /// Initialized each of the image views for each of the images in the swap
  /// chain
  /// </summary>
  void create_image_views(device *dev);

  VkSwapchainKHR get() { return vulkan_swap_chain; };

  std::vector<VkImage> &get_images() { return images; };
  std::vector<VkImageView> &get_image_views() {
    return image_views;
  };

  VkFormat get_image_format() { return image_format; };
  VkExtent2D get_extent() { return extent; };


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

  GLFWwindow *glfw_window;
  VkSurfaceKHR vulkan_surface;

  VkDevice vulkan_device;
  VkPhysicalDevice vulkan_physical_device;

  VkSwapchainKHR vulkan_swap_chain;

  std::vector<VkImage> images;
  VkFormat image_format;

  VkExtent2D extent;

  std::vector<VkImageView> image_views;
};
} // namespace eng