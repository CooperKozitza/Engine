#pragma once

#include <algorithm>

#include "device.hpp"
#include "shader.hpp"
#include "swap_chain.hpp"

namespace eng {
class graphics_pipeline {
public:
  graphics_pipeline();
  ~graphics_pipeline();

  void set_shader(const char *file_path, shader_type type);

  void create_graphics_pipeline(device *dev, swap_chain *sc);
  void create_render_pass(device *dev, swap_chain *sc);

  bool required_shaders_set();

  VkPipeline get() { return vulkan_graphics_pipeline; };
  VkPipelineLayout get_layout() { return pipeline_layout; };
  VkRenderPass get_render_pass() { return render_pass; };

private:
  VkRenderPass render_pass;
  VkPipelineLayout pipeline_layout;
  VkPipeline vulkan_graphics_pipeline;

  VkDevice vulkan_device;

  std::vector<shader *> shaders;
};
} // namespace eng