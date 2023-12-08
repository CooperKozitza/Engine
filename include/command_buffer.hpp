#pragma once

#include "device.hpp"
#include "framebuffer.hpp"
#include "graphics_pipeline.hpp"
#include "swap_chain.hpp"
#include "vertex_buffer.hpp"

namespace eng {
struct command_buffer_options {
  graphics_pipeline *graphics_pipeline;
  swap_chain *swap_chain;
  framebuffer *framebuffer;
  vertex_buffer *vertex_buffer;
  color clear_color;
};

class command_buffers {
public:
  command_buffers();
  ~command_buffers();

  void create_command_pool(device *dev);
  void create_command_buffers(device *dev, uint32_t count);

  void record_command_buffer(command_buffer_options &opts, uint32_t image_index,
                             uint32_t buffer_index);

  std::vector<VkCommandBuffer> &get() { return m_command_buffers; };
  VkCommandBuffer &get(size_t buffer_index) {
    return m_command_buffers[buffer_index];
  };

private:
  VkDevice m_device;

  VkCommandPool m_command_pool;
  std::vector<VkCommandBuffer> m_command_buffers;
};
} // namespace eng