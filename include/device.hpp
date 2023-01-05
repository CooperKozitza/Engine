#pragma once

#include <map>
#include <optional>
#include <memory>
#include <set>

#include "instance.hpp"
#include "surface.hpp"

namespace eng {
struct buffer_create_options {
  VkDeviceSize size;
  VkBufferUsageFlags usage;
  VkMemoryPropertyFlags properties;
  VkBuffer *buffer;
  VkDeviceMemory *buffer_memory;
};

struct queue_family_indices {
  std::optional<uint32_t> graphics_family;
  std::optional<uint32_t> present_family;

  bool is_complete() {
    return graphics_family.has_value() && present_family.has_value();
  }
};

struct swap_chain_support_details {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class device {
public:
  device();
  ~device();

  void create_device(instance &inst, surface &surf);

  VkQueue &get_graphics_queue() { return m_graphics_queue; };
  VkQueue &get_present_queue() { return m_present_queue; };

  swap_chain_support_details &get_swap_chain_support_details() {
    return m_swap_chain_support_details;
  };
  queue_family_indices &get_queue_family_indices() {
    return m_queue_family_indices;
  };

  VkPhysicalDevice &get_physical_device() { return m_physical_device; };
  VkDevice &get_device() { return m_device; };

  uint32_t find_memory_type(uint32_t type_filter,
                            VkMemoryPropertyFlags properties);

  void create_buffer(buffer_create_options &opts);
  void copy_buffer(command_pool &pool, VkBuffer src, VkBuffer dst,
                   VkDeviceSize size);

private:
  /// <summary>
  /// Finds the available queue with both graphics and present support
  /// </summary>
  /// <param name="device">The physical device to check</param>
  /// <returns>A struct with the indices of the queues</returns>
  queue_family_indices find_queue_families(VkPhysicalDevice device);

  /// <summary>
  /// Used in determining device suitability, checks if the device can support a
  /// swap chain which is neccessarry for a graphics pipline of any kind with
  /// Vulkan
  /// </summary>
  /// <param name="device">The physical device to check</param>
  /// <returns>A struct with the details of the swap chain the device can
  /// support</returns>
  swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device);

  /// <summary>
  /// Gathers the available devices and picks the best one
  /// </summary>
  void pick_physical_device();

  /// <summary>
  /// Returns a score depending on the devices ability to be suitable
  /// </summary>
  /// <param name="device">The physical device to check</param>
  /// <returns>The score where 0 is a non-suitable device</returns>
  int rate_physical_device_suitability(VkPhysicalDevice device);

  /// <summary>
  /// Creates the graphics and present queue
  /// </summary>
  /// <param name="queue_priority">The priority of the queue; a value from 0.0f
  /// to 1.0f</param>
  void create_logical_device(const float queue_priority = 1.0f);

  /// <summary>
  /// Checks if REQUIRED_DEVICE_EXTENTIONS are available
  /// </summary>
  /// <param name="device">The physical device to check for support on</param>
  bool check_device_extention_support(VkPhysicalDevice device);

  VkInstance m_instance;
  VkSurfaceKHR m_surface;

  VkPhysicalDevice m_physical_device;
  VkDevice m_device;

  VkQueue m_graphics_queue, m_present_queue;

  swap_chain_support_details m_swap_chain_support_details;
  queue_family_indices m_queue_family_indices;
};
} // namespace eng