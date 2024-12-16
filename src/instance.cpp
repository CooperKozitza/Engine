#include "../include/instance.hpp"

eng::result<eng::instance> eng::instance::create_instance(const char* application_name, bool debug_layers) {
    if (debug_layers && !check_validation_layer_support()) {
        return eng::result<eng::instance>::error("Validation layers requested but not available.");
    }

    VkApplicationInfo application_info{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = application_name;
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "No Engine";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_0;

    uint32_t extention_count = 0;
    const char** extentions = glfwGetRequiredInstanceExtensions(&extention_count);

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = extention_count;
    create_info.ppEnabledExtensionNames = extentions;

    if (debug_layers) {
        create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    else {
        create_info.enabledLayerCount = 0;
    }

    VkInstance handle = VK_NULL_HANDLE;
    if (vkCreateInstance(&create_info, nullptr, &handle) != VK_SUCCESS) {
        return eng::result<eng::instance>::error("Failed to create instance.");
    }

    return eng::result<eng::instance>::success(instance(handle, application_info));
}

eng::instance::instance() : instance_handle(VK_NULL_HANDLE), application_info() {}

eng::instance::instance(VkInstance instance_handle, VkApplicationInfo application_info)
    : instance_handle(instance_handle), application_info(application_info) {}

eng::instance::~instance() {
    if (instance_handle != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_handle, nullptr);
    }
}

eng::instance::instance(instance&& other) noexcept
    : instance_handle(std::exchange(other.instance_handle, VK_NULL_HANDLE)), application_info(other.application_info) {}

eng::instance& eng::instance::operator=(instance&& other) noexcept {
    if (this != &other) {
        if (instance_handle != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_handle, nullptr);
        }

        instance_handle = std::exchange(other.instance_handle, VK_NULL_HANDLE);
        application_info = other.application_info;
    }

    return *this;
}

bool eng::instance::check_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (const char* layer_name : eng::validation_layers) {
        bool layer_found = false;

        for (const VkLayerProperties& layer_properties : available_layers) {
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
