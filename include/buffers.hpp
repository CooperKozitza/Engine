#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <mutex>

#include "device.hpp"
#include "framebuffer.hpp"
#include "graphics_pipeline.hpp"

namespace eng {
struct command_buffer_options;

class command_pool {
public:
  command_pool();
  ~command_pool();

  void create_command_pool(device &dev);
  void create_command_buffers(device &dev, uint32_t count);

  std::vector<VkCommandBuffer> &get_command_buffers() {
    return m_command_buffers;
  };
  VkCommandBuffer &get_command_buffer(size_t buffer_index) {
    return m_command_buffers[buffer_index];
  };

  VkCommandPool get_pool() { return m_command_pool; };

private:
  VkDevice m_device;

  VkCommandPool m_command_pool;
  std::vector<VkCommandBuffer> m_command_buffers;
};

class uniform_buffer {
public:
  uniform_buffer();
  ~uniform_buffer();

  void create_uniform_buffer(device &dev);
  void update_uniform_buffer(uint32_t current_image,
                             uniform_buffer_object &ubo);

  VkBuffer &get_uniform_buffer() { return m_uniform_buffer; }

private:
  VkDevice m_device;

  VkBuffer m_uniform_buffer;
  VkDeviceMemory m_uniform_buffer_memory;
  void *m_uniform_buffer_mapped;

  std::mutex m_mutex;
};

class vertex_buffer {
public:
  vertex_buffer();
  ~vertex_buffer();

  void create_vertex_buffer(device *dev, command_pool *cmd_buffs);
  void create_index_buffer(device *dev, command_pool *cmd_buffs);

  void set_vertices(std::vector<vertex> &vertices);
  void set_indices(std::vector<uint16_t> &indices);

  VkBuffer &get_vertex_buffer() { return m_vertex_buffer; };
  VkBuffer &get_index_buffer() { return m_index_buffer; }

  size_t get_vertex_count() { return m_vertices.size(); }
  size_t get_index_count() { return m_indices.size(); }

private:
  VkDevice m_device;

  VkBuffer m_vertex_buffer;
  VkBuffer m_index_buffer;

  VkDeviceMemory m_vertex_buffer_memory;
  VkDeviceMemory m_index_buffer_memory;

  std::vector<vertex> m_vertices;
  std::vector<uint16_t> m_indices;

  std::mutex m_mutex;
};

class descriptor_pool {
public:
  descriptor_pool();
  ~descriptor_pool();

  void create_descriptor_pool(device &dev, uint32_t set_count);
  void create_descriptor_set(device &dev, pipeline &gp, uniform_buffer &ub);

  std::vector<VkDescriptorSet> &get_descriptor_sets() {
    return m_descriptor_sets;
  }
  VkDescriptorSet &get_descriptor_set(size_t set_index) {
    return m_descriptor_sets[set_index];
  }

  VkDescriptorPool &get_descriptor_pool() { return m_descriptor_pool; }

private:
  VkDevice m_device;

  VkDescriptorPool m_descriptor_pool;

  std::vector<VkDescriptorSet> m_descriptor_sets;
};

struct command_buffer_options {
  pipeline *graphics_pipeline;
  swap_chain *swap_chain;
  framebuffer *framebuffer;
  vertex_buffer *vertex_buffer;
  descriptor_pool *descriptor_set;

  glm::vec3 clear_color;
};
} // namespace eng