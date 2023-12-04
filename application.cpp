#include "application.hpp"

eng::application *eng::application::app;

std::mutex eng::application::creation_mutex;

eng::application *eng::application::create(const vec2<unsigned int> &res,
                                           const char *title) {
  std::lock_guard<std::mutex> guard(creation_mutex);
  if (!app) {
    app = new application(res, title);
  }

  return app;
}

eng::application::application(const vec2<unsigned int> res, const char *name)
    : win(new window(res, name)), name(name), physical_device(VK_NULL_HANDLE) {
  init_vulkan(name);

  if (ENABLE_VALIDATION_LAYERS) {
    setup_debug_messenger();
  }

  create_surface();
  pick_physical_device();
  create_logical_device();
}

void eng::application::init_vulkan(const char *application_name) {
  if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support()) {
    throw std::runtime_error("validation layers requested, but not available!");
  }

  VkApplicationInfo app_info{};

  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = application_name;
  app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.pEngineName = "No Engine";
  app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  app_info.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo create_info{};

  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  std::vector<const char *> extentions = get_required_extentions();

  create_info.enabledExtensionCount =
      static_cast<unsigned int>(extentions.size());
  create_info.ppEnabledExtensionNames = extentions.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
  if (ENABLE_VALIDATION_LAYERS) {
    create_info.enabledLayerCount =
        static_cast<uint32_t>(VALIDATION_LAYERS.size());
    create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

    populate_debug_messenger_create_info(debug_create_info);
    create_info.pNext =
        (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
  } else {
    create_info.enabledLayerCount = 0;
    create_info.pNext = nullptr;
  }

  if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }
}

void eng::application::pick_physical_device() {
  unsigned int device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  if (device_count == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  std::multimap<int, VkPhysicalDevice> device_suitability_map;
  for (const auto &device : devices) {
    int score = rate_physical_device_suitability(device);
    device_suitability_map.insert(std::make_pair(score, device));
  }

  if (device_suitability_map.rbegin()->first > 0) {
    physical_device = device_suitability_map.rbegin()->second;

    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(physical_device, &device_props);

    std::cout << "Selected Device: " << device_props.vendorID << ' '
              << device_props.deviceName << " (" << device_props.deviceID << ')'
              << std::endl;
  } else {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  if (physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }
}

int eng::application::rate_physical_device_suitability(
    VkPhysicalDevice device) {
  VkPhysicalDeviceProperties device_props;
  VkPhysicalDeviceFeatures device_feats;
  vkGetPhysicalDeviceProperties(device, &device_props);
  vkGetPhysicalDeviceFeatures(device, &device_feats);

  std::cout << "Found Device: " << device_props.vendorID << ' '
            << device_props.deviceName << " (" << device_props.deviceID << ')'
            << std::endl;

  int score = 0;

  // Discrete GPUs have a significant performance advantage
  if (device_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 1000;
  }

  // Maximum possible size of textures affects graphics quality
  score += device_props.limits.maxImageDimension2D;

  // Application can't function without geometry shaders
  if (!device_feats.geometryShader) {
    return 0;
  }

  // Application can't function without graphics queue family
  queue_family_indices indices = find_queue_families(device);
  if (!indices.is_complete()) {
    return 0;
  }

  return score;
}

eng::queue_family_indices
eng::application::find_queue_families(VkPhysicalDevice device) {
  eng::queue_family_indices indices;

  unsigned int queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           nullptr);

  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
                                           queue_families.data());

  int i = 0;
  for (const auto &queue_family : queue_families) {
    if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphics_family = i;
    }

    unsigned int present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);

    if (present_support) {
      indices.present_family = i;
    }

    if (indices.is_complete()) {
      break;
    }

    i++;
  }

  return indices;
}

void eng::application::create_logical_device(const float queue_priority) {
  queue_family_indices indices = find_queue_families(physical_device);

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
  std::set<unsigned int> unique_queue_families = {
      indices.graphics_family.value(), indices.present_family.value()};

  for (unsigned int queue_family : unique_queue_families) {
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = indices.graphics_family.value();
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    queue_create_infos.push_back(queue_create_info);
  }

  // leaving everything to false for now
  VkPhysicalDeviceFeatures device_features{};

  VkDeviceCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount =
      static_cast<uint32_t>(queue_create_infos.size());
  create_info.pQueueCreateInfos = queue_create_infos.data();
  create_info.pEnabledFeatures = &device_features;
  create_info.enabledExtensionCount = 0;
  create_info.enabledLayerCount = 0;

  if (vkCreateDevice(physical_device, &create_info, nullptr, &device) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(device, indices.graphics_family.value(), 0, &graphics_queue);
  std::cout << "Created Graphics Queue" << std::endl;
  vkGetDeviceQueue(device, indices.present_family.value(), 0, &graphics_queue);
  std::cout << "Created Present Queue" << std::endl;
}

void eng::application::create_surface() {
  GLFWwindow *glfw_window = win->get_glfw_window();
  if (glfwCreateWindowSurface(instance, glfw_window, nullptr, &surface) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create window surface!");
  }

  std::cout << "Created Window Surface" << std::endl;
}

bool eng::application::check_device_extention_support(VkPhysicalDevice device) {
  unsigned int extension_count;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       nullptr);

  std::vector<VkExtensionProperties> available_extensions(extension_count);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count,
                                       available_extensions.data());

  std::set<std::string> required_extensions(REQUIRED_DEVICE_EXTENTIONS.begin(),
                                            REQUIRED_DEVICE_EXTENTIONS.end());

  for (const auto &extension : available_extensions) {
    required_extensions.erase(extension.extensionName);
  }

  return required_extensions.empty();
}

bool eng::application::check_validation_layer_support() {
  unsigned int layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const char *layer_name : VALIDATION_LAYERS) {
    bool layer_found = false;

    for (const auto &layer_properties : available_layers) {
      if (strcmp(layer_name, layer_properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found) {
      return false;
    }
  }

  return true;
}

void eng::application::setup_debug_messenger() {
  if (!ENABLE_VALIDATION_LAYERS)
    return;

  VkDebugUtilsMessengerCreateInfoEXT create_info{};
  populate_debug_messenger_create_info(create_info);

  if (CreateDebugUtilsMessengerEXT(instance, &create_info, nullptr,
                                   &debug_messenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}

void eng::application::populate_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
  createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debug_callback;
}

std::vector<const char *> eng::application::get_required_extentions() {
  unsigned int glfw_extension_count = 0;
  const char **glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char *> extensions(glfw_extensions,
                                       glfw_extensions + glfw_extension_count);

  if (ENABLE_VALIDATION_LAYERS) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

eng::application::~application() {
  if (is_running()) {
    stop();
  }

  if (ENABLE_VALIDATION_LAYERS) {
    DestroyDebugUtilsMessengerEXT(instance, debug_messenger, nullptr);
  }

  vkDestroyInstance(instance, nullptr);

  vkDestroySurfaceKHR(instance, surface, nullptr);
  vkDestroyDevice(device, nullptr);

  delete win;
}

void eng::application::start() {
  running = true;
  main = std::thread([this] {
    while (!win->should_close() && is_running()) {
      win->poll_events();

      // main loop logic
    }
  });
}

void eng::application::stop() {
  running = false;
  main.join();
}

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugUtilsMessengerEXT *pDebugMessenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

  if (func != nullptr) {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
                                   VkDebugUtilsMessengerEXT debugMessenger,
                                   const VkAllocationCallbacks *pAllocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func != nullptr) {
    func(instance, debugMessenger, pAllocator);
  }
}
