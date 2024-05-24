#pragma once

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace eng {
struct uniform_buffer_object {
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 proj;

  static VkDescriptorSetLayoutBinding get_descriptor_set_layout_binding() {
    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_binding.pImmutableSamplers = nullptr; // Optional

    return layout_binding;
  }
};
} // namespace eng