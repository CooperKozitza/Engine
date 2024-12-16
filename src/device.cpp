#include "../include/device.hpp"

#include <map>

eng::result<VkPhysicalDevice> eng::device::pick_physical_device(eng::instance instance) {
    if (!instance.valid()) {
        return eng::result<VkPhysicalDevice>::error("Instance is not valid.");
    }

    VkInstance vulkan_instance = instance.get_vulkan_instance();

    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, nullptr);

    if (device_count == 0) {
        return eng::result<VkPhysicalDevice>::error("No devices found with Vulkan support.");
    }

    std::vector<VkPhysicalDevice> physical_devices(device_count);
    vkEnumeratePhysicalDevices(vulkan_instance, &device_count, physical_devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const VkPhysicalDevice& device : physical_devices) {
        if (is_device_suitable(device)) {
            int device_score = rate_device_suitability(device);
            candidates.insert(std::make_pair(device_score, device));
        }
    }

    if (candidates.begin()->first > 0) {
        return eng::result<VkPhysicalDevice>::success(candidates.begin()->second);
    }

    return eng::result<VkPhysicalDevice>::error("Failed to find suitable GPU.");
}

eng::result<VkDevice> eng::device::create_logical_device(VkPhysicalDevice physical_device, bool debug_layers) {
    queue_family_indices indices = find_queue_families(physical_device);

    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = indices.graphics_family.value();
    queue_create_info.queueCount = 1;

    // Vulkan lets you assign priorities to queues to influence the scheduling 
    // of command buffer execution using floating point numbers between 0.0 and
    // 1.0. This is required even if there is only a single queue
    float queuePriority = 1.0f;
    queue_create_info.pQueuePriorities = &queuePriority;

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queue_create_info;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &device_features;
    createInfo.enabledExtensionCount = 0; 

    if (debug_layers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(eng::validation_layers.size());
        createInfo.ppEnabledLayerNames = eng::validation_layers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    VkDevice device;

    if (vkCreateDevice(physical_device, &createInfo, nullptr, &device) == VK_SUCCESS) {
        return eng::result<VkDevice>::success(device);
    }

    return eng::result<VkDevice>::error("Failed to create logical device.");
}

eng::device::queue_family_indices eng::device::find_queue_families(VkPhysicalDevice physical_device) {
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

        if (indices.complete()) {
            break;
        }

        ++index;
    }

    return indices;
}

bool eng::device::is_device_suitable(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceProperties physical_device_properties;
    vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

    VkPhysicalDeviceFeatures physical_device_features;
    vkGetPhysicalDeviceFeatures(physical_device, &physical_device_features);

    eng::device::queue_family_indices indices = find_queue_families(physical_device);

    return physical_device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
        && physical_device_features.geometryShader
        && indices.complete();
}

int eng::device::rate_device_suitability(VkPhysicalDevice physical_device) {
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

eng::device::device() : physical_device_handle(VK_NULL_HANDLE), logical_device_handle(VK_NULL_HANDLE) {}

eng::device::device(VkPhysicalDevice physical_device_handle)
