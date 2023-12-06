#pragma once

#include <vulkan/vulkan.h>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <thread>
#include <vector>
#include <algorithm>

#include "vectors.hpp"
#include "window.hpp"

#ifndef _DEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif // !_DEBUG

const std::vector<const char *> VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> REQUIRED_DEVICE_EXTENTIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

/// <summary>
/// Creates a debug messenger
/// </summary>
/// <param name="instance">The Vulkan instance to attach to</param>
/// <param name="pCreateInfo">The parameters for creation</param>
/// <param name="pDebugMessenger">The callback</param>
VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger);

/// <summary>
/// Destroys a debug messenger
/// </summary>
/// <param name="instance">The Vulkan instance to attach to</param>
/// <param name="debugMessenger">The messenger to destroy</param>
void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator);

namespace eng {
struct queue_family_indices {
  std::optional<unsigned int> graphics_family;
  std::optional<unsigned int> present_family;

  bool is_complete() {
    return graphics_family.has_value() && present_family.has_value();
  }
};

struct swap_chain_support_details {
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> present_modes;
};

class application {
public:
  static application *create(const vec2<unsigned int> &res, const char *title);
  static application *get() { return app; }

  application(const application &) = delete;
  const application &operator=(const application &) = delete;

  ~application();

  void start();
  void stop();

  bool is_running() { return running; }

private:
  application(const vec2<unsigned int> res, const char *name);
  static application *app;

  // helper funcs:

  /// <summary>
  /// Calls the functions neccessary to initalize Vulkan
  /// </summary>
  /// <param name="application_name">The name of the application and
  /// window</param>
  void init_vulkan(const char *application_name);

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
  /// Finds the available queue with both graphics and present support
  /// </summary>
  /// <param name="device">The physical device to check</param>
  /// <returns>A struct with the indices of the queues</returns>
  eng::queue_family_indices find_queue_families(VkPhysicalDevice device);

  /// <summary>
  /// Creates the graphics and present queue
  /// </summary>
  /// <param name="queue_priority">The priority of the queue; a value from 0.0f
  /// to 1.0f</param>
  void create_logical_device(const float queue_priority = 1.0f);

  /// <summary>
  /// Uses the window to create a VkSurfaceKHR
  /// </summary>
  void create_surface();

  /// <summary>
  /// Checks if REQUIRED_DEVICE_EXTENTIONS are available
  /// </summary>
  /// <param name="device">The physical device to check for support on</param>
  bool check_device_extention_support(VkPhysicalDevice device);

  /// <summary>
  /// Checks if the validation layer is supported on the current instance
  /// </summary>
  bool check_validation_layer_support();

  /// <summary>
  /// Sets up the debug messenger if validation layers are enabled
  /// </summary>
  void setup_debug_messenger();

  /// <summary>
  /// Populates create_info with the correct values for the validation layer
  /// debug messenger
  /// </summary>
  /// <param name="createInfo">The create_info to modify</param>
  void populate_debug_messenger_create_info(
      VkDebugUtilsMessengerCreateInfoEXT &create_info);

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
  /// Gets a list of the neccessarry Vulkan API extentions
  /// </summary>
  /// <returns>The names of the extentions</returns>
  std::vector<const char *> get_required_extentions();

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
  
  /// <summary>
  /// Creates the swap chain
  /// </summary>
  void create_swap_chain();

  /// <summary>
  /// Validation layer callback function
  /// </summary>
  /// <param name="message_severity">The severity of the debug message</param>
  /// <param name="message_type">The type of message</param>
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                 VkDebugUtilsMessageTypeFlagsEXT message_type,
                 const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
                 void *p_user_data) {

    std::cerr << "Validation Layer: \n";
    std::cerr << "- " << p_callback_data->pMessage << '\n' << std::endl;

    return VK_FALSE;
  }

  // prevents two threads from simultaneously creating an application
  // instance
  static std::mutex creation_mutex;

  // flag for whether or not the application is running (used to stop thread)
  std::atomic<bool> running;

  // the thread where the main loop runs, defined in start()
  std::thread main;

  // the name of the application
  const char *name;

  // the glfw window instance
  window_details win_details;
  window *win;

  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkSurfaceKHR surface;
  VkDebugUtilsMessengerEXT debug_messenger;
  VkQueue graphics_queue;
  VkQueue present_queue;
  VkSwapchainKHR swap_chain;
  std::vector<VkImage> swap_chain_images;
  VkFormat swap_chain_image_format;
  VkExtent2D swap_chain_extent;
};
} // namespace eng