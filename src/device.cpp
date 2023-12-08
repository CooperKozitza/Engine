#include "../include/device.hpp"

eng::device::device()
    : m_instance(VK_NULL_HANDLE), m_surface(VK_NULL_HANDLE),
      m_physical_device(VK_NULL_HANDLE), m_device(VK_NULL_HANDLE),
      m_graphics_queue(VK_NULL_HANDLE), m_present_queue(VK_NULL_HANDLE) {}

eng::device::~device() {
  if (m_device != VK_NULL_HANDLE) {
    vkDestroyDevice(m_device, nullptr);
  }
}

void eng::device::create_device(instance *inst, surface *surf) {
  m_instance = inst->get();
  m_surface = surf->get();

  pick_physical_device();
  create_logical_device();
}

void eng::device::pick_physical_device() {
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

  if (device_count == 0) {
    throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

  std::multimap<int, VkPhysicalDevice> device_suitability_map;
  for (const auto &device : devices) {
    int score = rate_physical_device_suitability(device);
    device_suitability_map.insert(std::make_pair(score, device));
  }

  if (device_suitability_map.rbegin()->first > 0) {
    m_physical_device = device_suitability_map.rbegin()->second;

    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(m_physical_device, &device_props);

    std::cout << "Selected Device: " << device_props.vendorID << ' '
              << device_props.deviceName << " (" << device_props.deviceID << ')'
              << std::endl;
  } else {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  if (m_physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error("failed to find a suitable GPU!");
  }

  m_queue_family_indices = find_queue_families(m_physical_device);
  m_swap_chain_support_details = query_swap_chain_support(m_physical_device);
}

int eng::device::rate_physical_device_suitability(VkPhysicalDevice device) {
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

  bool extensions_supported = check_device_extention_support(device);

  bool swap_chain_adequate = false;
  if (extensions_supported) {
    swap_chain_support_details swap_chain_support =
        query_swap_chain_support(device);
    swap_chain_adequate = !swap_chain_support.formats.empty() &&
                          !swap_chain_support.present_modes.empty();
  }

  if (!(extensions_supported && swap_chain_adequate)) {
    std::cerr << "Device does not support either swap chains or the required "
                 "extentions!"
              << std::endl;
    return 0;
  }

  return score;
}

uint32_t eng::device::find_memory_type(uint32_t type_filter,
                                       VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties mem_properties;
  vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

eng::device::queue_family_indices
eng::device::find_queue_families(VkPhysicalDevice device) {
  queue_family_indices indices;

  uint32_t queue_family_count = 0;
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

    uint32_t present_support = false;
    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface,
                                         &present_support);

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

eng::device::swap_chain_support_details
eng::device::query_swap_chain_support(VkPhysicalDevice device) {
  swap_chain_support_details details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface,
                                            &details.capabilities);

  uint32_t format_count;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count,
                                       nullptr);

  if (format_count != 0) {
    details.formats.resize(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &format_count,
                                         details.formats.data());
  }

  uint32_t present_mode_count;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface,
                                            &present_mode_count, nullptr);

  if (present_mode_count != 0) {
    details.present_modes.resize(present_mode_count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface,
                                              &present_mode_count,
                                              details.present_modes.data());
  }

  return details;
}

void eng::device::create_logical_device(const float queue_priority) {
  queue_family_indices indices = find_queue_families(m_physical_device);

  std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
  std::set<uint32_t> unique_queue_families = {
      indices.graphics_family.value(), indices.present_family.value()};

  for (uint32_t queue_family : unique_queue_families) {
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

  create_info.enabledExtensionCount =
      static_cast<uint32_t>(REQUIRED_DEVICE_EXTENTIONS.size());
  create_info.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTENTIONS.data();

  if (ENABLE_VALIDATION_LAYERS) {
    create_info.enabledLayerCount =
        static_cast<uint32_t>(VALIDATION_LAYERS.size());
    create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();
  } else {
    create_info.enabledLayerCount = 0;
  }

  if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create logical device!");
  }

  vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0,
                   &m_graphics_queue);
  std::cout << "Created Graphics Queue" << std::endl;
  vkGetDeviceQueue(m_device, indices.present_family.value(), 0,
                   &m_present_queue);
  std::cout << "Created Present Queue" << std::endl;
}

bool eng::device::check_device_extention_support(VkPhysicalDevice device) {
  uint32_t extension_count;
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