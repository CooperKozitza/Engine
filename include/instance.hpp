#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include "../include/result.hpp"

namespace eng {
    const std::vector<const char*> validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    class instance {
    public:
        static result<instance> create_instance(const char* application_name, bool debug_layers = false);

        instance();
        ~instance();

        instance(const instance&) = delete;
        instance& operator=(const instance&) = delete;

        instance(instance&& other) noexcept;
        instance& operator=(instance&& other) noexcept;

        bool valid() const { return instance_handle != VK_NULL_HANDLE; }

        VkInstance get_vulkan_instance() const { return instance_handle; }
        VkApplicationInfo get_vulkan_application_info() const { return application_info; }
    private:
        instance(VkInstance instance_handle, VkApplicationInfo application_info);

        static bool check_validation_layer_support();

        VkInstance instance_handle;
        VkApplicationInfo application_info;
    };
}
