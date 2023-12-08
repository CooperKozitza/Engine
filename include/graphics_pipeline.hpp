#pragma once

#include <algorithm>

#include <glm/glm.hpp>

#include "device.hpp"
#include "shader.hpp"
#include "swap_chain.hpp"
#include "vertex.hpp"

namespace eng {
class graphics_pipeline {
public:
  graphics_pipeline();
  ~graphics_pipeline();

  void set_shader(const char *file_path, shader_type type);

  void create_graphics_pipeline(device *dev, swap_chain *sc);
  void create_render_pass(device *dev, swap_chain *sc);

  bool required_shaders_set();

  VkPipeline &get() { return m_graphics_pipeline; };
  VkPipelineLayout &get_layout() { return m_pipeline_layout; };
  VkRenderPass &get_render_pass() { return m_render_pass; };

private:
  VkRenderPass m_render_pass;
  VkPipelineLayout m_pipeline_layout;
  VkPipeline m_graphics_pipeline;

  VkDevice m_device;

  std::vector<shader *> m_shaders;
};
} // namespace eng