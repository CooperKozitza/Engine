#include "../include/command_buffer.hpp"

eng::command_buffer::command_buffer()
    : m_device(VK_NULL_HANDLE), m_command_pool(VK_NULL_HANDLE),
      m_command_buffer(VK_NULL_HANDLE) {}

eng::command_buffer::~command_buffer() {
  if (m_device != VK_NULL_HANDLE && m_command_pool != VK_NULL_HANDLE) {
    vkDestroyCommandPool(m_device, m_command_pool, nullptr);
  }
}

void eng::command_buffer::create_command_pool(device *dev) {
  m_device = dev->get();

  device::queue_family_indices queue_family_indices =
      dev->get_queue_family_indices();

  VkCommandPoolCreateInfo pool_info{};
  pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  pool_info.queueFamilyIndex = queue_family_indices.graphics_family.value();

  if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }
}

void eng::command_buffer::create_command_buffer(device *dev) {
  m_device = dev->get();

  VkCommandBufferAllocateInfo alloc_info{};
  alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  alloc_info.commandPool = m_command_pool;
  alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  alloc_info.commandBufferCount = 1;

  if (vkAllocateCommandBuffers(m_device, &alloc_info, &m_command_buffer) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate command buffers!");
  }
}

void eng::command_buffer::record_command_buffer(command_buffer_options &opts,
                                                uint32_t image_index) {
  VkCommandBufferBeginInfo begin_info{};
  begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(m_command_buffer, &begin_info) != VK_SUCCESS) {
    throw std::runtime_error("failed to begin recording command buffer!");
  }

  VkRenderPassBeginInfo render_pass_info{};
  render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  render_pass_info.renderPass = opts.graphics_pipeline->get_render_pass();
  render_pass_info.framebuffer = opts.framebuffer->get_framebuffer(image_index);
  render_pass_info.renderArea.offset = {0, 0};
  render_pass_info.renderArea.extent = opts.swap_chain->get_extent();

  VkClearValue clear_color = {{{opts.clear_color.r, opts.clear_color.g,
                               opts.clear_color.b, opts.clear_color.a}}};
  render_pass_info.clearValueCount = 1;
  render_pass_info.pClearValues = &clear_color;

  vkCmdBeginRenderPass(m_command_buffer, &render_pass_info,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    opts.graphics_pipeline->get());

  VkViewport viewport{};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = (float)opts.swap_chain->get_extent().width;
  viewport.height = (float)opts.swap_chain->get_extent().height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(m_command_buffer, 0, 1, &viewport);

  VkRect2D scissor{};
  scissor.offset = {0, 0};
  scissor.extent = opts.swap_chain->get_extent();
  vkCmdSetScissor(m_command_buffer, 0, 1, &scissor);

  vkCmdDraw(m_command_buffer, 3, 1, 0, 0);

  vkCmdEndRenderPass(m_command_buffer);

  if (vkEndCommandBuffer(m_command_buffer) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer!");
  }
}
