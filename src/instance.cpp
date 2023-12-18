#include "../include/instance.hpp"

const std::vector<const char *> eng::VALIDATION_LAYERS = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> eng::REQUIRED_DEVICE_EXTENTIONS = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

eng::instance::instance()
    : m_instance(VK_NULL_HANDLE), m_debug_messenger(VK_NULL_HANDLE) {}

eng::instance::~instance() {
  if (ENABLE_VALIDATION_LAYERS) {
    destroy_debug_utils_messenger_ext(m_instance, m_debug_messenger, nullptr);
  }

  vkDestroyInstance(m_instance, nullptr);
}

void eng::instance::create_instance(const char *application_name) {
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

  std::cout << "Required Extentions: " << std::endl;
  for (const char *ext : extentions) {
    std::cout << "- " << ext << '\n';
  }

  create_info.enabledExtensionCount =
      static_cast<uint32_t>(extentions.size());
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

  if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS) {
    throw std::runtime_error("failed to create instance!");
  }

  setup_debug_messenger();

  std::cout << "Created Vulkan Instance" << std::endl;
}

VkInstance &eng::instance::get_instance() { return m_instance; }

VkDebugUtilsMessengerEXT &eng::instance::get_debug_messenger() {
  return m_debug_messenger;
}

VkResult eng::instance::create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT *p_create_info,
    const VkAllocationCallbacks *p_allocator,
    VkDebugUtilsMessengerEXT *p_debug_messenger) {
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkCreateDebugUtilsMessengerEXT");

  if (func != nullptr) {
    return func(instance, p_create_info, p_allocator, p_debug_messenger);
  } else {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void eng::instance::destroy_debug_utils_messenger_ext(
    VkInstance instance, VkDebugUtilsMessengerEXT m_debug_messenger,
    const VkAllocationCallbacks *p_allocator) {
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT");

  if (func != nullptr) {
    func(instance, m_debug_messenger, p_allocator);
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL eng::instance::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
    void *p_user_data) {
  if (message_severity < MINIMUM_VALIDATION_LAYER_MESSAGE_SEVERITY) {
    return VK_FALSE;
  }

  std::cerr << "Validation Layer: \n";
  std::cerr << "- " << p_callback_data->pMessage << '\n' << std::endl;

  return VK_FALSE;
}

bool eng::instance::check_validation_layer_support() {
  uint32_t layer_count;
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

std::vector<const char *> eng::instance::get_required_extentions() {
  uint32_t glfw_extension_count = 0;
  const char **glfw_extensions;
  glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

  std::vector<const char *> extensions(glfw_extensions,
                                       glfw_extensions + glfw_extension_count);

  if (ENABLE_VALIDATION_LAYERS) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  return extensions;
}

void eng::instance::populate_debug_messenger_create_info(
    VkDebugUtilsMessengerCreateInfoEXT &create_info) {
  create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = debug_callback;
}

void eng::instance::setup_debug_messenger() {
  if (!ENABLE_VALIDATION_LAYERS)
    return;

  VkDebugUtilsMessengerCreateInfoEXT create_info{};
  populate_debug_messenger_create_info(create_info);

  if (create_debug_utils_messenger_ext(m_instance, &create_info, nullptr,
                                       &m_debug_messenger) != VK_SUCCESS) {
    throw std::runtime_error("failed to set up debug messenger!");
  }
}
