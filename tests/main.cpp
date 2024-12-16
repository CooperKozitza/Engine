#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <iostream>

#include "device.hpp"

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // try to create a instance and device

    std::cout << "Creating instance!\n";
    eng::result<eng::instance> instance = eng::instance::create_instance("Vulkan", window, true);

    if (instance.is_error()) {
        std::cerr << "Failed to create instance: " << instance.error_message() << '\n';
        return 1;
    }

    std::cout << "Creating device!\n";
    eng::result<eng::device> device = eng::device::create_device(instance.unwrap(), window, true);

    if (device.is_error()) {
        std::cerr << "Failed to create device: " << device.error_message() << '\n';
        return 1;
    }

    std::cout << extensionCount << " extensions supported\n";

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}
