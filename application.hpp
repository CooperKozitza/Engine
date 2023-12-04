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

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger);

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
  void init_vulkan(const char *application_name);
  void pick_physical_device();
  int rate_physical_device_suitability(VkPhysicalDevice device);
  eng::queue_family_indices find_queue_families(VkPhysicalDevice device);
  void create_logical_device(const float queue_priority = 1.0f);
  void create_surface();
  bool check_device_extention_support(VkPhysicalDevice device);
  bool check_validation_layer_support();
  void setup_debug_messenger();
  void populate_debug_messenger_create_info(
      VkDebugUtilsMessengerCreateInfoEXT &createInfo);

  std::vector<const char *> get_required_extentions();

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                 VkDebugUtilsMessageTypeFlagsEXT message_type,
                 const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
                 void *p_user_data) {

    std::cerr << "Validation Layer: " << p_callback_data->pMessage << std::endl;

    return VK_FALSE;
  }

  // prevents two threads from simultaneously creating an application
  // instance
  static std::mutex creation_mutex;

  // flag for whether or not the application is running (used to stop
  // thread)
  std::atomic<bool> running;
  // the thread where the main loop runs, defined in start()
  std::thread main;

  // the name of the application
  const char *name;

  // the glfw window instance
  window *win;

  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkSurfaceKHR surface;
  VkDebugUtilsMessengerEXT debug_messenger;
  VkQueue graphics_queue;
  VkQueue present_queue;
};
} // namespace eng