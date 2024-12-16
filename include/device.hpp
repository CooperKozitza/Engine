#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "instance.hpp"

#include <optional>

namespace eng {
    class device {
    public:
        static result<device> create_device(instance instance, bool debug_layers = false);

        device();
        ~device();

        device(const device&) = delete;
        device& operator=(const device&) = delete;

        device(device&& other) noexcept;
        device& operator=(device&& other) noexcept;

        bool valid() const { return logical_device_handle != VK_NULL_HANDLE; }

        VkDevice get_vulkan_logical_device() const { return logical_device_handle; }
    private:
        struct queue_family_indices {
            std::optional<uint32_t> graphics_family;

            bool complete() const noexcept { return graphics_family.has_value(); }
        };

        device(VkPhysicalDevice physical_device_handle, VkDevice logical_deivice_handle);

        static result<VkPhysicalDevice> pick_physical_device(VkInstance instance);
        static result<VkDevice> create_logical_device(VkPhysicalDevice physical_device, bool debug_layers = false);

        static queue_family_indices find_queue_families(VkPhysicalDevice physical_device);

        static bool is_device_suitable(VkPhysicalDevice physical_device);
        static int rate_device_suitability(VkPhysicalDevice physical_device);

        VkDevice logical_device_handle;
    };
}
