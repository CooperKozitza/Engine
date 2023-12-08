#pragma once

#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>

namespace eng {
#ifndef _DEBUG
constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif // !_DEBUG

constexpr VkDebugUtilsMessageSeverityFlagBitsEXT
    MINIMUM_VALIDATION_LAYER_MESSAGE_SEVERITY =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

/// <summary>
/// The enabled validation layers
/// </summary>
extern const std::vector<const char *> VALIDATION_LAYERS;

/// <summary>
/// These extentions are required for the functionality of the application, and
/// therefore are required to exist on any suitable device
/// </summary>
extern const std::vector<const char *> REQUIRED_DEVICE_EXTENTIONS;

class instance {
public:
  instance();
  ~instance();

  /// <summary>
  /// Calls the functions neccessary to initalize Vulkan
  /// </summary>
  /// <param name="application_name">The name of the application and
  /// window</param>
  void create_instance(const char *application_name);

  VkInstance get();
  VkDebugUtilsMessengerEXT get_debug_messenger();

private:
  /// <summary>
  /// Creates a debug messenger
  /// </summary>
  /// <param name="instance">The Vulkan instance to attach to</param>
  /// <param name="pCreateInfo">The parameters for creation</param>
  /// <param name="pDebugMessenger">The callback</param>
  VkResult create_debug_utils_messenger_ext(
      VkInstance instance,
      const VkDebugUtilsMessengerCreateInfoEXT *p_create_info,
      const VkAllocationCallbacks *p_allocator,
      VkDebugUtilsMessengerEXT *p_debug_messenger);

  /// <summary>
  /// Destroys a debug messenger
  /// </summary>
  /// <param name="instance">The Vulkan instance to attach to</param>
  /// <param name="debugMessenger">The messenger to destroy</param>
  void
  destroy_debug_utils_messenger_ext(VkInstance instance,
                                    VkDebugUtilsMessengerEXT debug_messenger,
                                    const VkAllocationCallbacks *p_allocator);

  /// <summary>
  /// Validation layer callback function
  /// </summary>
  /// <param name="message_severity">The severity of the debug message</param>
  /// <param name="message_type">The type of message</param>
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                 VkDebugUtilsMessageTypeFlagsEXT message_type,
                 const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
                 void *p_user_data);

  /// <summary>
  /// Checks if the validation layer is supported on the current instance
  /// </summary>
  bool check_validation_layer_support();

  /// <summary>
  /// Gets a list of the neccessarry Vulkan API extentions
  /// </summary>
  /// <returns>The names of the extentions</returns>
  std::vector<const char *> get_required_extentions();

  /// <summary>
  /// Populates create_info with the correct values for the validation layer
  /// debug messenger
  /// </summary>
  /// <param name="createInfo">The create_info to modify</param>
  void populate_debug_messenger_create_info(
      VkDebugUtilsMessengerCreateInfoEXT &create_info);

  /// <summary>
  /// Sets up the debug messenger if validation layers are enabled
  /// </summary>
  void setup_debug_messenger();

  VkInstance m_instance;
  VkDebugUtilsMessengerEXT debug_messenger;
};
} // namespace eng
