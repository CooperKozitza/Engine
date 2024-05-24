#pragma once

#include <array>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

namespace eng {
struct vertex {
  glm::vec3 pos;
  glm::vec3 color;

  static VkVertexInputBindingDescription get_binding_properties() {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
  }

  static std::array<VkVertexInputAttributeDescription, 2>
      get_attribute_description() {
    std::array<VkVertexInputAttributeDescription, 2> attribute_description{};
    attribute_description[0].binding = 0;
    attribute_description[0].location = 0;
    attribute_description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_description[0].offset = offsetof(vertex, pos);

    attribute_description[1].binding = 0;
    attribute_description[1].location = 1;
    attribute_description[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attribute_description[1].offset = offsetof(vertex, color);

    return attribute_description;
  }
};
} // namespace eng