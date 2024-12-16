#include <cstdint>
#include <vector>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "instance.hpp"

#include <optional>

namespace eng {
    const std::vector<const char *> device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    class device {
    public:
        static result<device> create_device(instance& instance, GLFWwindow* window, bool debug_layers = false);

        device();
        ~device();

        device(const device&) = delete;
        device& operator=(const device&) = delete;

        device(device&& other) noexcept;
        device& operator=(device&& other) noexcept;

        bool valid() const { return logical_device_handle != VK_NULL_HANDLE; }

        VkDevice get_vulkan_logical_device() const { return logical_device_handle; }
        VkSwapchainKHR get_vulkan_swap_chain() const { return swap_chain_handle; }
    private:
        struct queue_family_indices {
            std::optional<uint32_t> graphics_family;
            std::optional<uint32_t> present_family;

            bool complete() const noexcept { return graphics_family.has_value(); }
        };

        struct swap_chain_support_details {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };

        device(VkDevice logical_deivice_handle, VkQueue present_queue_handle, VkSwapchainKHR swap_chain_handle);

        static result<VkPhysicalDevice> pick_physical_device(VkInstance instance, VkSurfaceKHR surface);
        static result<VkDevice> create_logical_device(VkPhysicalDevice physical_device, VkSurfaceKHR surface, bool debug_layers = false);
        static result<VkSwapchainKHR> create_swap_chain(VkPhysicalDevice physical_device, VkDevice logical_device, VkSurfaceKHR surface, GLFWwindow* window);

        static result<queue_family_indices> find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

        static bool is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
        static int rate_device_suitability(VkPhysicalDevice physical_device);
        static bool check_device_extension_support(VkPhysicalDevice physical_device);

        static swap_chain_support_details query_swap_chain_support(VkPhysicalDevice physical_device, VkSurfaceKHR surface);

        static VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats);
        static VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes);
        static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

        VkDevice logical_device_handle;
        VkQueue present_queue_handle;
        VkSwapchainKHR swap_chain_handle;
    };
}
