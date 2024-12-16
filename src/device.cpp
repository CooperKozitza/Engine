#include "../include/device.hpp"
#include "GLFW/glfw3.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <map>
#include <utility>
#include <vector>
#include <set>
#include <vulkan/vulkan_core.h>

eng::result<VkPhysicalDevice> eng::device::pick_physical_device(VkInstance instance, VkSurfaceKHR surface) {
    if (instance == VK_NULL_HANDLE) {
        return eng::result<VkPhysicalDevice>::error("Invalid Vulkan instance.");
    }

    if (surface == VK_NULL_HANDLE) {
        return eng::result<VkPhysicalDevice>::error("Invalid Vulkan surface.");
    }

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

    if (device_count == 0) {
        return eng::result<VkPhysicalDevice>::error("No devices found with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const VkPhysicalDevice& device : physical_devices) {
        if (is_device_suitable(device, surface)) {
            int device_score = rate_device_suitability(device);
            candidates.insert(std::make_pair(device_score, device));
        }
    }

    if (candidates.size() > 0 && candidates.begin()->first > 0) {
        return eng::result<VkPhysicalDevice>::success(candidates.begin()->second);
    }

    return eng::result<VkPhysicalDevice>::error("Failed to find suitable GPU.");
}

eng::result<VkDevice> eng::device::create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, bool debug_layers) {
    if (physical_device == VK_NULL_HANDLE) {
        return eng::result<VkDevice>::error("Invalid Vulkan instance.");
    }

    if (surface == VK_NULL_HANDLE) {
        return eng::result<VkDevice>::error("Invalid Vulkan surface.");
    }

    eng::result<eng::device::queue_family_indices> indices_result = find_queue_families(physical_device, surface);

    if (indices_result.is_error()) {
        std::string error_message = "Error while finding queue indices: " + std::string(indices_result.error_message());

        return eng::result<VkDevice>::error(error_message.c_str());
    }

    eng::device::queue_family_indices indices = indices_result.unwrap();

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    std::set<uint32_t> unique_queue_families = { indices.graphics_family.value(), indices.present_family.value() };

    float queue_priority = 1.0f;
    for (uint32_t queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queue_priority;

        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = queue_create_infos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    createInfo.pEnabledFeatures = &device_features;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(eng::device_extensions.size()); 
    createInfo.ppEnabledExtensionNames = eng::device_extensions.data();

    if (debug_layers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(eng::validation_layers.size());
        createInfo.ppEnabledLayerNames = eng::validation_layers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    VkDevice device;

    if (vkCreateDevice(physical_device, &createInfo, nullptr, &device) != VK_SUCCESS) {
        return eng::result<VkDevice>::error("Failed to create logical device.");
    }

    return eng::result<VkDevice>::success(device);
}

eng::result<eng::device::queue_family_indices> eng::device::find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    if (physical_device == VK_NULL_HANDLE) {
        return eng::result<eng::device::queue_family_indices>::error("Invalid Vulkan instance.");
    }

    eng::device::queue_family_indices indices;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_family_properties(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

    int index = 0;
    for (const VkQueueFamilyProperties& queue_family : queue_family_properties) {
        if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = index;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, index, surface, &present_support);

        if (present_support) {
            indices.present_family = index;
        }

        if (indices.complete()) {
            break;
        }

        ++index;
    }

    return eng::result<eng::device::queue_family_indices>::success(indices);
}

bool eng::device::is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    if (physical_device == VK_NULL_HANDLE) {
        throw std::invalid_argument("Invalid Vulkan instance.");
    }

    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

    VkPhysicalDeviceFeatures physical_device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

    eng::result<eng::device::queue_family_indices> indices = find_queue_families(physical_device, surface);

    if (indices.is_error()) {
        return false;
    }

    bool extensions_supported = check_device_extension_support(physical_device);
    bool swap_chain_adequate = false;

    if (extensions_supported) {
        eng::device::swap_chain_support_details swap_chain_support = query_swap_chain_support(physical_device, surface);

        swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
    }

    return physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        && swap_chain_adequate
        && physical_device_features.geometryShader
        && indices.unwrap().complete();
}

bool eng::device::check_device_extension_support(VkPhysicalDevice physical_device) {
    if (physical_device == VK_NULL_HANDLE) {
        throw std::invalid_argument("Invalid Vulkan instance.");
    }

    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, available_extensions.data());

    std::set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

    for (const VkExtensionProperties& extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    return required_extensions.empty();
}

int eng::device::rate_device_suitability(VkPhysicalDevice physical_device) {
    if (physical_device == VK_NULL_HANDLE) {
        throw std::invalid_argument("Invalid Vulkan instance.");
    }

    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

    VkPhysicalDeviceFeatures physical_device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

    if (!physical_device_features.geometryShader) {
        return 0;
    }

    int score = 0;

    if (physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += physical_device_properties.limits.maxImageDimension2D;

    return score;
}

eng::device::device() : logical_device_handle(VK_NULL_HANDLE) {}

eng::device::device(VkDevice logical_device_handle, VkQueue present_queue_handle, VkSwapchainKHR swap_chain_handle) 
    : logical_device_handle(logical_device_handle),
    present_queue_handle(present_queue_handle),
    swap_chain_handle(swap_chain_handle) {}

eng::device::~device() {
    if (logical_device_handle != VK_NULL_HANDLE) {
        if (swap_chain_handle != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(logical_device_handle, swap_chain_handle, nullptr);
        }

        vkDestroyDevice(logical_device_handle, nullptr);
    }
}

eng::device::device(eng::device&& other) noexcept
    : logical_device_handle(std::exchange(other.logical_device_handle, VK_NULL_HANDLE)),
    present_queue_handle(std::exchange(other.present_queue_handle, VK_NULL_HANDLE)){}

eng::device& eng::device::operator=(eng::device&& other) noexcept {
    if (this != &other) {
        if (logical_device_handle != VK_NULL_HANDLE) {
            vkDestroyDevice(logical_device_handle, nullptr);
        }

        logical_device_handle = std::exchange(other.logical_device_handle, VK_NULL_HANDLE);
        present_queue_handle = std::exchange(other.present_queue_handle, VK_NULL_HANDLE);
    }

    return *this;
}

eng::result<eng::device> eng::device::create_device(eng::instance& instance, GLFWwindow* window, bool debug_layers) {
    if (!instance.valid()) {
        return eng::result<eng::device>::error("Invalid instance.");
    }

    if (window == nullptr) {
        return eng::result<eng::device>::error("Invalid window.");
    }

    VkInstance instance_handle = instance.get_vulkan_instance();
    VkSurfaceKHR surface_handle = instance.get_vulkan_surface();

    eng::result<VkPhysicalDevice> physical_device_result = pick_physical_device(instance_handle, surface_handle);

    if (physical_device_result.is_error()) {
        return eng::result<eng::device>::error(physical_device_result.error_message());
    }

    VkPhysicalDevice physical_device = physical_device_result.unwrap();

    eng::result<VkDevice> logical_device_result = create_logical_device(physical_device, surface_handle, debug_layers);

    if (logical_device_result.is_error()) {
        return eng::result<eng::device>::error(logical_device_result.error_message());
    }

    VkDevice logical_device = logical_device_result.unwrap();

    eng::result<eng::device::queue_family_indices> indices_result = find_queue_families(physical_device, surface_handle);

    if (indices_result.is_error()) {
        std::string error_message = "Error while finding queue indices: " + std::string(indices_result.error_message());

        return eng::result<eng::device>::error(error_message.c_str());
    }

    eng::device::queue_family_indices indices = indices_result.unwrap();

    VkQueue present_queue;
    vkGetDeviceQueue(logical_device, indices.present_family.value(), 0, &present_queue);

    eng::result<VkSwapchainKHR> swap_chain_result = create_swap_chain(physical_device, logical_device, surface_handle, window);

    if (swap_chain_result.is_error()) {
        return eng::result<eng::device>::error(swap_chain_result.error_message());
    }

    VkSwapchainKHR swap_chain = swap_chain_result.unwrap();

    return eng::result<eng::device>::success(device(logical_device, present_queue, swap_chain));
}

eng::device::swap_chain_support_details eng::device::query_swap_chain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
    if (physical_device == VK_NULL_HANDLE) {
        throw std::invalid_argument("Invalid Vulkan instance.");
    }

    if (surface == VK_NULL_HANDLE) {
        throw std::invalid_argument("Invalid Vulkan surface.");
    }

    swap_chain_support_details details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);

    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, details.present_modes.data());
    }

    return details;
}

VkSurfaceFormatKHR eng::device::choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
    for (const VkSurfaceFormatKHR& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }

    return available_formats[0];
}

VkPresentModeKHR eng::device::choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
    for (const VkPresentModeKHR& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D eng::device::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actual_extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actual_extent;
}

eng::result<VkSwapchainKHR> eng::device::create_swap_chain(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface, GLFWwindow* window) {
    if (physical_device == VK_NULL_HANDLE) {
        return eng::result<VkSwapchainKHR>::error("Invalid Vulkan instance.");
    }

    if (logical_device == VK_NULL_HANDLE) {
        return eng::result<VkSwapchainKHR>::error("Invalid Vulkan logical device.");
    }

    if (surface == VK_NULL_HANDLE) {
        return eng::result<VkSwapchainKHR>::error("Invalid Vulkan surface.");
    }

    if (window == nullptr) {
        return eng::result<VkSwapchainKHR>::error("Invalid window.");
    }

    eng::device::swap_chain_support_details swap_chain_support = query_swap_chain_support(physical_device, surface);

    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
    VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
    VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities, window);

    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;

    if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
        image_count = swap_chain_support.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    eng::result<eng::device::queue_family_indices> indices_result = find_queue_families(physical_device, surface);

    if (indices_result.is_error()) {
        std::string error_message = "Error while finding queue indices: " + std::string(indices_result.error_message());

        return eng::result<VkSwapchainKHR>::error(error_message.c_str());
    }

    eng::device::queue_family_indices indices = indices_result.unwrap();

    uint32_t queue_family_indices[] = { indices.graphics_family.value(), indices.present_family.value() };

    if (indices.graphics_family != indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = nullptr;
    }

    create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkSwapchainKHR swap_chain_handle;
    if (vkCreateSwapchainKHR(logical_device, &create_info, nullptr, &swap_chain_handle) != VK_SUCCESS) {
        return eng::result<VkSwapchainKHR>::error("Failed to create swap chain.");
    }

    return eng::result<VkSwapchainKHR>::success(swap_chain_handle);
}
