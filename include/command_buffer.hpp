#pragma once

#include "device.hpp"
#include "graphics_pipeline.hpp"
#include "swap_chain.hpp"
#include "framebuffer.hpp"

namespace eng {
struct command_buffer_options {
  graphics_pipeline *graphics_pipeline;
  swap_chain *swap_chain;
  framebuffer *framebuffer;
  color clear_color;
};

class command_buffer {
public:
  command_buffer();
  ~command_buffer();

  void create_command_pool(device *dev);
  void create_command_buffer(device *dev);

  void record_command_buffer(command_buffer_options &opts, uint32_t image_index);

  VkCommandBuffer &get() { return m_command_buffer; };

private:
  VkDevice m_device;

  VkCommandPool m_command_pool;
  VkCommandBuffer m_command_buffer;
};
} // namespace eng