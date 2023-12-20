#pragma once

#include <algorithm>

#include <glm/glm.hpp>

#include "device.hpp"
#include "shader.hpp"
#include "swap_chain.hpp"
#include "vertex.hpp"
#include "uniform_buffer_object.hpp"

namespace eng {
class pipeline {
public:
  pipeline();
  ~pipeline();

  void add_shader(const char *file_path, shader_type type);

  void create_descriptor_set_layout(device &dev);
  void create_graphics_pipeline(device &dev, swap_chain &sc);
  void create_render_pass(device &dev, swap_chain &sc);

  bool required_shaders_set();

  VkRenderPass &get_render_pass() { return m_render_pass; }
  VkDescriptorSetLayout &get_descriptor_set_layout() {
    return m_descriptor_set_layout;
  }
  VkPipelineLayout &get_pipeline_layout() { return m_pipeline_layout; }
  VkPipeline &get_pipeline() { return m_graphics_pipeline; }

private:
  VkRenderPass m_render_pass;
  VkDescriptorSetLayout m_descriptor_set_layout;
  VkPipelineLayout m_pipeline_layout;
  VkPipeline m_graphics_pipeline;

  VkDevice m_device;

  std::vector<std::unique_ptr<shader>> m_shaders;
};
} // namespace eng